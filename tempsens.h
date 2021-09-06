#ifndef TEMPSENS_H
#define TEMPSENS_H

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include <QVector>

//This is a separate thread class running independent of the main GUI thread

class tempsens
{
public:
    tempsens();

    tempsens(pthread_mutex_t* mut);
    ~tempsens();

    int testdata;
    int file_error;
    QVector<float> temperature;

    /// Start the thread
    bool Start(void);

    /// POSIX id (handle) for the thread
    pthread_t threadId_;

    /// Function seen by POSIX as thread function
    static void* ThreadWrapper(void* data);

    private:
    /// Mutex reference
    pthread_mutex_t *mut_;

    /// This function does the thread work
    void Thread(void);


};

#endif // TEMPSENS_H
