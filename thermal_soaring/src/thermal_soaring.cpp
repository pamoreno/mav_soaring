//  October/2019, Auterion AG, Jaeyoung Lim, jaeyoung@auterion.com

#include "thermal_soaring/thermal_soaring.h"

using namespace Eigen;
using namespace std;
//Constructor
ThermalSoaring::ThermalSoaring(const ros::NodeHandle& nh, const ros::NodeHandle& nh_private):
  nh_(nh),
  nh_private_(nh_private),
  thermal_estimator_(nh, nh_private) {

  cmdloop_timer_ = nh_.createTimer(ros::Duration(0.01), &ThermalSoaring::cmdloopCallback, this); // Define timer for constant loop rate
  statusloop_timer_ = nh_.createTimer(ros::Duration(1), &ThermalSoaring::statusloopCallback, this); // Define timer for constant loop rate

  mavpose_sub_ = nh_.subscribe("/mavros/local_position/pose", 1, &ThermalSoaring::mavposeCallback, this,ros::TransportHints().tcpNoDelay());
  mavtwist_sub_ = nh_.subscribe("/mavros/local_position/velocity_local", 1, &ThermalSoaring::mavtwistCallback, this,ros::TransportHints().tcpNoDelay());
  windest_sub_ = nh_.subscribe("/mavros/windestimation", 1, &ThermalSoaring::windestimationCallback, this, ros::TransportHints().tcpNoDelay());

  setpointraw_pub_ = nh_.advertise<mavros_msgs::PositionTarget>("/mavros/setpoint_raw/local", 1);

}
ThermalSoaring::~ThermalSoaring() {
  //Destructor
}

void ThermalSoaring::cmdloopCallback(const ros::TimerEvent& event){
  
  if(thermal_estimator_.IsInThermal()){
    target_position_ = thermal_estimator_.getThermalPosition();

  } else {
    //TODO: Add exploration logic
  }

  PubPositionSetpointRaw();

}

void ThermalSoaring::statusloopCallback(const ros::TimerEvent& event){

  thermal_estimator_.UpdateState(mavPos_, mavVel_);

  //TODO: Evaluate waypoint to decide if it is reachable with glide slope
  // If the mission waypoint is unreachable, search for thermal
  // If mission point is reachable, move to mission point
  // If altitude is too high, exit thermal

  //TODO: If not in thermal, keep track if the launch point is reachable. If not return to Home
  //This should be optional, in case the vehicle is powered
  //TODO: Visualize thermal
}

void ThermalSoaring::PubPositionSetpointRaw(){
  mavros_msgs::PositionTarget msg;
  msg.header.stamp = ros::Time::now();
  msg.header.frame_id = "map";
  msg.type_mask = 0x3000;
  msg.position.x = target_position_(0);
  msg.position.y = target_position_(1);
  msg.position.z = target_position_(2);
  setpointraw_pub_.publish(msg);

}

void ThermalSoaring::mavposeCallback(const geometry_msgs::PoseStamped& msg){
  mavPos_(0) = msg.pose.position.x;
  mavPos_(1) = msg.pose.position.y;
  mavPos_(2) = msg.pose.position.z;
  mavAtt_(0) = msg.pose.orientation.w;
  mavAtt_(1) = msg.pose.orientation.x;
  mavAtt_(2) = msg.pose.orientation.y;
  mavAtt_(3) = msg.pose.orientation.z;

}

void ThermalSoaring::mavtwistCallback(const geometry_msgs::TwistStamped& msg){ 
  mavVel_(0) = msg.twist.linear.x;
  mavVel_(1) = msg.twist.linear.y;
  mavVel_(2) = msg.twist.linear.z;
  mavRate_(0) = msg.twist.angular.x;
  mavRate_(1) = msg.twist.angular.y;
  mavRate_(2) = msg.twist.angular.z;
  
}

void ThermalSoaring::windestimationCallback(const geometry_msgs::TwistWithCovarianceStamped& msg){
  wind_velocity_(0) = msg.twist.twist.linear.x;
  wind_velocity_(1) = msg.twist.twist.linear.y;
  wind_velocity_(2) = msg.twist.twist.linear.z;

}