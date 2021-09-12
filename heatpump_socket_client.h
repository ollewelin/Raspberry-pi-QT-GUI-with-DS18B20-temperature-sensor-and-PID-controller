#ifndef HEATPUMP_SOCKET_CLIENT_H
#define HEATPUMP_SOCKET_CLIENT_H

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <QVector>

//This is a separate thread class running independent of the main GUI QT thread

class heatpump_socket_client
{
public:
    heatpump_socket_client();

    heatpump_socket_client(pthread_mutex_t* mut);
    ~heatpump_socket_client();

    int file_error;
    // *********** Shared memory **************
    //This data have to be handle with mutex to multithread shared memory
    QVector<int> socket_send;
    QVector<int> socket_receive;
    int socket_initialized;
    int nrofheatpsignals;
    // *********** End of shared memory **************
    /// Start the thread
    bool Start(void);

    /// POSIX id (handle) for the thread
    pthread_t threadId_;

    /// Function seen by POSIX as thread function
    static void* ThreadWrapper(void* data);

    private:
    /// Mutex reference
    pthread_mutex_t *mut_;
    QVector<int> payload_local;

    /// This function does the thread work
    void Thread(void);


};

#endif // HEATPUMP_SOCKET_CLIENT_H
