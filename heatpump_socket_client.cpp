#include "heatpump_socket_client.h"
#include<vector>
#include <iostream>
/* client.c */

#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_MSG_SIZE 10

#pragma pack(1)

    typedef struct payload_t {
        int socket_data[SOCKET_MSG_SIZE];
    } payload;

#pragma pack()

heatpump_socket_client::~heatpump_socket_client()
{
    pthread_cancel(threadId_);
    printf("heatpump_socket_client is deleted.\n");
};

heatpump_socket_client::heatpump_socket_client(pthread_mutex_t* mut)
     :mut_(mut)

{
    printf("Constructor heatpump_socket_client thread class \n");
    socket_send.clear();
    socket_receive.clear();
    payload_local.clear();
    file_error = 0;
    socket_initialized = 0;
    nrofheatpsignals = SOCKET_MSG_SIZE;
};



void sendMsg(int sock, void* msg, uint32_t msgsize)
{
    if (write(sock, msg, msgsize) < 0)
    {
        printf("Can't send message.\n");
        close(sock);
        exit(1);
    }
    printf("Message sent (%d bytes).\n", msgsize);
    return;
}

/// Function seen by POSIX as thread function
void* heatpump_socket_client::ThreadWrapper(void* data)
{
    // data is a pointer to a Thread, so we can call the
    // real thread function

    (static_cast<heatpump_socket_client*>(data))->Thread();//Here is where the magic happend to connect this thread object wrapper to the tempsens::Thread(void) funtion below
    return 0;
}


void heatpump_socket_client::Thread(void)
{
    printf("heatpump_socket_client thread program started\n");
    file_error =0;
    pthread_mutex_lock(mut_);
    socket_initialized = 0;
    pthread_mutex_unlock(mut_);
    for(int i=0;i<SOCKET_MSG_SIZE;i++)
    {
        socket_send.push_back(0);
        socket_receive.push_back(0);
        payload_local.push_back(0);
    }
    for(int i=0;i<SOCKET_MSG_SIZE;i++)
    {
        //socket_send[i] = i -1000;//Test data
        socket_send[i] = 0;
        if(i==1){
            socket_send[i] = 20;
        }
    }

    const int PORT = 2300;
    const char* SERVERNAME = "localhost";
    uint32_t BUFFSIZE = sizeof(payload_local) * payload_local.size();
    uint8_t buff[BUFFSIZE];
    int local_int=0;
    uint8_t buff_li[sizeof (local_int)];

    int sock;
    int nread;
    time_t t;

    srand((unsigned) time(&t));

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERNAME, &server_address.sin_addr);
    server_address.sin_port = htons(PORT);

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: Socket creation failed\n");
        //return 1;
        file_error = 1;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("ERROR: Unable to connect to the python script tinytuya heatpump server\n");
        printf("start the heatpump_server.py\n");
        printf("before strat this program \n");
        printf("By do this command on raspberry pi \n");
        printf("$ python3  pheatpump_server.py\n");
        //return 1;
        file_error = 1;
    }
    if(file_error == 1)
    {
        pthread_mutex_lock(mut_);
        socket_initialized = -1;
        pthread_mutex_unlock(mut_);
    }
    else {
        pthread_mutex_lock(mut_);
        socket_initialized = 1;
        pthread_mutex_unlock(mut_);
    }
    printf("Connected to %s\n", SERVERNAME);
    if(socket_initialized == 1){
        while(1){
            for(int i = 0; i < SOCKET_MSG_SIZE; i++) {
                pthread_mutex_lock(mut_);
                payload_local[i] = socket_send[i];
                pthread_mutex_unlock(mut_);
            }
            //payload data;
            for(int i=0;i<payload_local.size();i++)
            {
                //Copy to socket data buffert
                local_int = payload_local[i];
                memcpy(buff_li, &local_int, 4);
                for(int k=0;k<4;k++){
                    buff[i*4 + k] = buff_li[k];
                }
            }
            sendMsg(sock, &buff, BUFFSIZE);
            bzero(buff, BUFFSIZE);
            nread = read(sock, buff, BUFFSIZE);
            for(int i=0;i<payload_local.size();i++)
            {
                //Copy from socket data buffert
                for(int k=0;k<4;k++){
                    buff_li[k] = buff[i*4 + k];
                }
                memcpy(&local_int, &buff_li, 4);
                payload_local[i] = local_int;
            }


            for(int i = 0; i < SOCKET_MSG_SIZE; i++) {
                pthread_mutex_lock(mut_);
                socket_receive[i] = payload_local[i];
                pthread_mutex_unlock(mut_);
                printf("socket_receive[%d]=%d\n", i,socket_receive[i]);
            }
            printf("Socket Thread set to sleep 100ms\n");
            usleep(100000);//Sleep inside this thread.
        }
        // close the socket
        close(sock);

    }
}

bool heatpump_socket_client::Start(void)
{
   // Start the thread, send it the this pointer (points to this class
  // instance)

  //pthread_create() takes 4 arguments.
  //The first argument is a pointer to thread_id which is set by this function.
  //The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
  //The third argument is name of function to be executed for the thread to be created.
  //The fourth argument is used to pass arguments to the function.
  printf("Create and start the heatpump_socket_client thread, this is a separated thread from the QT thread \n");
  return (pthread_create(&threadId_,    // Pointer to the thread handle
                         NULL,          // Optional ptr to thread settings
                         ThreadWrapper, // Thread function
                         this) == 0);   // Argument passed to thread func
}



/*
 *
 *

    //Debugg
    payload_local.push_back(75313);
    payload_local.push_back(-3152);
    printf("payload_local.size = %d\n", payload_local.size());
    printf("sizeof payload_local[0] = %d\n", sizeof (payload_local[0]));
    printf("sizeof payload_local = %d\n", sizeof (payload_local));
    int BUFFSIZE2 = sizeof(payload_local) * payload_local.size();
    printf("BUFFSIZE2=%d\n", BUFFSIZE2);
    uint8_t buff[BUFFSIZE2];
    uint32_t local_int=0;
    uint8_t buff_li[sizeof (local_int)];


    for(int i=0;i<payload_local.size();i++)
    {
       //Copy to socket data buffert
       local_int = payload_local[i];
       printf("debug x1\n");
       memcpy(buff_li, &local_int, 4);
       for(int k=0;k<4;k++){
           buff[i*4 + k] = buff_li[k];
       }
       printf("debug x k=%d\n", i);
    }

    for(int i=0;i<BUFFSIZE2;i++)
    {
        printf("buff[%d]=%d\n", i, buff[i]);
    }
    buff[0] = 161;
    for(int i=0;i<BUFFSIZE2;i++)
    {
        printf("buff[%d]=%d\n", i, buff[i]);
    }


    for(int i=0;i<payload_local.size();i++)
    {
        //Copy from socket data buffert

        for(int k=0;k<4;k++){
            buff_li[k] = buff[i*4 + k];
        }
        memcpy(&local_int, &buff_li, 4);
        payload_local[i] = local_int;
    }


    printf("debug\n");

    printf("test convertion QVectror and std::vector\n");
      std::vector<int> stdvect;
      QVector<int> qvect;
        stdvect.clear();
        qvect.clear();
        stdvect.push_back(1);
        stdvect.push_back(2);
        qvect.push_back(0);
        qvect.push_back(0);

        qvect = QVector<int>::fromStdVector(stdvect);
        qvect[0] = qvect[0] * 5;
        qvect[1] = qvect[1] * 5;
        stdvect = qvect.toStdVector();
        std::cout << "stdvect[0] = " << stdvect[0] << "\n";
        std::cout << "stdvect[1] = " << stdvect[1] << "\n";

*/
