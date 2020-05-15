/**
 * @file offb_node.cpp
 * @brief Offboard control example node, written with MAVROS version 0.19.x, PX4 Pro Flight
 * Stack and tested in Gazebo SITL
 */

#include <ros/ros.h>
#include <geometry_msgs/PoseStamped.h>
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>

#include <sstream>
void new_pos_Callback(const geometry_msgs::PoseStamped::ConstPtr& msg);
mavros_msgs::State current_state;
void state_cb(const mavros_msgs::State::ConstPtr& msg){
    current_state = *msg;
}
geometry_msgs::PoseStamped pose;

int main(int argc, char **argv)
{
    ros::init(argc, argv, "offb_node1");
    ros::NodeHandle nh3;

    ros::Subscriber state_sub = nh3.subscribe<mavros_msgs::State>
            ("uav2/mavros/state", 10, state_cb);
    ros::Publisher local_pos_pub = nh3.advertise<geometry_msgs::PoseStamped>
            ("uav2/mavros/setpoint_position/local", 10);
    ros::ServiceClient arming_client = nh3.serviceClient<mavros_msgs::CommandBool>
            ("uav2/mavros/cmd/arming");
    ros::ServiceClient set_mode_client = nh3.serviceClient<mavros_msgs::SetMode>
            ("uav2/mavros/set_mode");
    
    ros::Subscriber sub = nh3.subscribe("my_position", 10, new_pos_Callback);        
    ros::Rate rate(20.0);

    // wait for FCU connection
    while(ros::ok() && !current_state.connected){
        ros::spinOnce();
        rate.sleep();
    }

    pose.pose.position.x = 2.25;
    pose.pose.position.y = 2.25;
    pose.pose.position.z = 2;

    //send a few setpoints before starting
    for(int i = 100; ros::ok() && i > 0; --i){
        local_pos_pub.publish(pose);
        ros::spinOnce();
        rate.sleep();
    }

    mavros_msgs::SetMode offb_set_mode;
    offb_set_mode.request.custom_mode = "OFFBOARD";

    mavros_msgs::CommandBool arm_cmd;
    arm_cmd.request.value = true;

    ros::Time last_request = ros::Time::now();

    while(ros::ok()){
        if( current_state.mode != "OFFBOARD" &&
            (ros::Time::now() - last_request > ros::Duration(5.0))){
            if( set_mode_client.call(offb_set_mode) &&
                offb_set_mode.response.mode_sent){
                ROS_INFO("Offboard enabled");
            }
            last_request = ros::Time::now();
        } else {
            if( !current_state.armed &&
                (ros::Time::now() - last_request > ros::Duration(5.0))){
                if( arming_client.call(arm_cmd) &&
                    arm_cmd.response.success){
                    ROS_INFO("Vehicle armed");
                }
                last_request = ros::Time::now();
            }
        }

        local_pos_pub.publish(pose);

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
void new_pos_Callback(const geometry_msgs::PoseStamped::ConstPtr& msg){
    pose=*msg;
    pose.pose.position.x +=1; 
    pose.pose.position.y +=1 ;
    

    ROS_INFO("new position recived: ");
}
