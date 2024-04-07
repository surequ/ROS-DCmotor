
#include <ros/ros.h>
#include <cstdio>
#include <geometry_msgs/TransformStamped.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>
#include <ros/ros.h>
#include <geometry_msgs/TransformStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Pose.h>
#include <turtlesim/Spawn.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

class BroadCasterTest
{
public:
  float out;
  BroadCasterTest() : nh_()
  {
    timer_ = nh_.createTimer(ros::Duration(0.01), [&](const ros::TimerEvent& e) {
    out = broadcast_dynamic_tf();
      counter_++;
    });
  }
  float get_rota(void)
  {
    return out;
  }
//ros::Duration()で周期決定
//その後はコールバック関数
private:
  void broadcast_static_tf(void)
  {
    geometry_msgs::TransformStamped static_transformStamped;
    static_transformStamped.header.stamp = ros::Time::now();
    static_transformStamped.header.frame_id = "base_link";
    static_transformStamped.child_frame_id = "static_tf";
    static_transformStamped.transform.translation.x = 0.0;
    static_transformStamped.transform.translation.y = 0.0;
    static_transformStamped.transform.translation.z = 0.3;
    //クォータニオン作成
    tf2::Quaternion quat;
    quat.setRPY(0.0, 0.0, 0.0);
    static_transformStamped.transform.rotation.x = quat.x();
    static_transformStamped.transform.rotation.y = quat.y();
    static_transformStamped.transform.rotation.z = quat.z();
    static_transformStamped.transform.rotation.w = quat.w();
    //ブロードキャスト
    static_br_.sendTransform(static_transformStamped);
  }
  float broadcast_dynamic_tf(void)
  {
    geometry_msgs::TransformStamped transformStamped;
    transformStamped.header.stamp = ros::Time::now();
    transformStamped.header.frame_id = "base_link";
    transformStamped.child_frame_id = "round_link";
    transformStamped.transform.translation.x = 0.0;
    transformStamped.transform.translation.y = 0.0;
    transformStamped.transform.translation.z = 0.0;
    tf2::Quaternion q;
    q.setRPY(counter_*0.004, 0, 0);
    transformStamped.transform.rotation.x = q.x();
    transformStamped.transform.rotation.y = q.y();
    transformStamped.transform.rotation.z = q.z();
    transformStamped.transform.rotation.w = q.w();
    dynamic_br_.sendTransform(transformStamped);
    return transformStamped.transform.rotation.x;
  }




  ros::NodeHandle nh_;
  ros::Timer timer_;
  tf2_ros::TransformBroadcaster dynamic_br_;
  tf2_ros::StaticTransformBroadcaster static_br_;
  int counter_;
};

class ListenTest
{
public:
  ListenTest() : nh_(), tfBuffer_(), tfListener_(tfBuffer_)
  {
    timer_ = nh_.createTimer(ros::Duration(0.1), [&](const ros::TimerEvent& e) {
      geometry_msgs::TransformStamped transformStamped;
      try
      {
        transformStamped = tfBuffer_.lookupTransform("base_link", "round_link", ros::Time(0));
      }
      catch (tf2::TransformException& ex)
      {
        ROS_WARN("%s", ex.what());
        return;
      }

      auto& trans = transformStamped.transform.translation;
      auto& rota = transformStamped.transform.rotation;
      ROS_INFO("world->rotation: %f %f %f", rota.x, rota.y, rota.z);
      geometry_msgs::Pose object_d, object_w;
      object_d.position.z = 1.0;
      object_d.orientation.w = 1.0;
      tf2::doTransform(object_d, object_w, transformStamped);
      });
  }

private:
  ros::NodeHandle nh_;
  ros::Timer timer_;
  tf2_ros::Buffer tfBuffer_;
  tf2_ros::TransformListener tfListener_;
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "controller_broad");
  //周期ごとの
  BroadCasterTest broadcast_test;
  float out_rota = broadcast_test.get_rota();
  ROS_INFO("%f",out_rota);
  ListenTest listen_test;
  ros::spin();
  return 0;
};