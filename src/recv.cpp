#include "ros/ros.h"
#include "std_msgs/String.h"
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
 
void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  try
  {
    cv::Mat image = cv_bridge::toCvShare(msg, "bgr8")->image;
    cv::imshow("view", image);
//    cv::imwrite("/home/limz/1.jpg", image);

  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}
 
int main(int argc, char **argv)
{
  ros::init(argc, argv, "img_listener");
  ros::NodeHandle nh;
  cv::namedWindow("view");
  cv::startWindowThread();
  image_transport::ImageTransport it(nh);
  image_transport::Subscriber sub = it.subscribe("/usb_cam/image_raw",1,imageCallback);
  ros::spin();
  cv::destroyWindow("view");
 
  return 0;
}
