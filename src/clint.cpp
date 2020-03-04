//socket headfile
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include <arpa/inet.h>
#include <unistd.h>    //close(client_socket);
 
//opencv headfile
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>
#include <chrono>

#include "ros/ros.h"
#include "std_msgs/String.h"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>

using  namespace cv;
using  namespace std;
 
#define HELLO_WORLD_SERVER_PORT   7754
#define BUFFER_SIZE 1024

int client_socket;
void imageCallback(const sensor_msgs::ImageConstPtr& msg)
{
    Mat s_img;
    try{
        s_img = cv_bridge::toCvShare(msg, "bgr8")->image;
    }
    catch (cv_bridge::Exception& e){
        ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
    }
    
    /********************* send image to serve ****************************/
    auto start = std::chrono::system_clock::now();
    
    //compress image
    vector<uchar> encode_img;
    std::vector<int> quality;
	quality.push_back(CV_IMWRITE_JPEG_QUALITY);
	quality.push_back(100); 
	imencode(".jpg", s_img, encode_img, quality);

    //get send_buffer
    int encode_img_size = encode_img.size();
    int s_img_size = s_img.rows*s_img.cols*3;
    printf("filesize is %d, width*hight*3 is %d\n", encode_img_size, s_img_size);
    uchar* send_buffer = new uchar[encode_img.size()];
    
    copy(encode_img.begin(), encode_img.end(), send_buffer);
    
    int toSend = encode_img_size, receive  = 0, finished = 0;
    char char_len[10];

    //2.send image length
    sprintf(char_len, "%d", toSend);
    send(client_socket, char_len, 10, 0);
    
    //3.send image data
    while(toSend  >  0){
        int size = min(toSend, 1000);
        if((receive = send(client_socket, send_buffer + finished, size, 0))){
            if(receive==-1){
                printf ("receive error");
                break;
            }
            else{
                toSend -= receive;
                finished += receive; 
            }
        }
    }
    printf("finish\n");
    auto end = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << elapsed.count() <<"ms" << '\n';
}

int main(int argc, char *argv[])
{
    ros::init(argc, argv, "img_listener");
    ros::NodeHandle nh;
    
    cv::startWindowThread();
    image_transport::ImageTransport it(nh);
    image_transport::Subscriber sub = it.subscribe("/usb_cam/image_raw", 1, imageCallback);

    if (argc != 2){
        printf("Usage: %s ServerIPAddress\n", argv[0]);
        exit(1);
    }
    
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr)); 
    client_addr.sin_family = AF_INET;    
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);  
 
    client_socket = socket(AF_INET,SOCK_STREAM,0);
    if( client_socket < 0){
        printf("Create Socket Failed!\n");
        exit(1);
    }

    if( bind(client_socket,(struct sockaddr*)&client_addr,sizeof(client_addr))){
        printf("Client Bind Port Failed!\n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_aton(argv[1],&server_addr.sin_addr) == 0){
        printf("Server IP Address Error!\n");
        exit(1);
    }
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);
    if(connect(client_socket,(struct sockaddr*)&server_addr, server_addr_length) < 0){
        printf("Can Not Connect To %s!\n",argv[1]);
        exit(1);
    }
    printf("success connect To %s!\n",argv[1]);

    //connect test
    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);

    int length = recv(client_socket,buffer,BUFFER_SIZE,0);
    if(length < 0){
        printf("Recieve Data From Server %s Failed!\n", argv[1]);
        exit(1);
    }
    printf("\n%s\n",buffer);
    bzero(buffer,BUFFER_SIZE);

    strcpy(buffer,"Hello, World! From Client\n");
    int send_flag=send(client_socket,buffer,BUFFER_SIZE,0);
    if(!send_flag)
        printf("send error\n");
    printf("send success\n");
    
    ros::spin();

    close(client_socket);
    printf("close socket\n");
    return 0;
}
