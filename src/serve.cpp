//本文件是服务器的代码
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdio.h>        // for QDebug
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <time.h>                //for time_t and time
#include <unistd.h>
//#include <printf>
 
#define HELLO_WORLD_SERVER_PORT 7754
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024

#include <iostream>
#include <vector>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
using  namespace cv;
using  namespace std;
int main(int argc, char *argv[])
{
    struct sockaddr_in server_addr;
    bzero(&server_addr,sizeof(server_addr)); 
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    
    int server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket < 0){
        printf("Create Socket Failed!");
        exit(1);
    }
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        printf("Server Bind Port : %d Failed!", HELLO_WORLD_SERVER_PORT);
        exit(1);
    }
    listen(server_socket, LENGTH_OF_LISTEN_QUEUE);
    
    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
        int new_server_socket = accept(server_socket,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            printf("Server Accept Failed!\n");
            break;
        }
        
        //test connect
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        
        strcpy(buffer,"Hello,World! from serve!\n");
        send(new_server_socket, buffer, BUFFER_SIZE,0);
        bzero(buffer, BUFFER_SIZE);
        
        length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);
        printf("%s\n",buffer);

        while(1){
            char char_len[10];
            long read_length, image_length;
            int finished = 0;
            recv(new_server_socket, char_len, 10, 0);
            image_length = read_length = atoi(char_len);
            printf("received read_length is %ld\n", read_length);
            
            uchar* file_buffer = new uchar[read_length];
            while(read_length > 1000){
                int receive = recv(new_server_socket, file_buffer + finished, 1000, 0);
                read_length -= receive;
                finished += receive;
            }
            read_length = recv(new_server_socket, file_buffer + finished, 1000, 0);
     
            std::vector<uchar> decode(file_buffer, file_buffer+image_length);
        
		    Mat image = imdecode(decode, CV_LOAD_IMAGE_COLOR); //image decode
		    imshow("image", image);
		    char c = waitKey(1);
		    switch(c){
		        case 'q':
		            return 0;
		            break;
		        default:
		            break;
		    }
        }
        
        close(new_server_socket);
    }
    close(server_socket);
    return 0;
}
