#include "tempsens.h"

//https://www.waveshare.com/wiki/Raspberry_Pi_Tutorial_Series:_1-Wire_DS18B20_Sensor
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <QVector>

#define ONE_WIRE_DEVICE_PATH "/sys/bus/w1/devices/"

//This code working with Raspberry Pi one wire to DS18B20 tempsensors

tempsens::~tempsens()
{
    pthread_cancel(threadId_);
    printf("tempsens is deleted.\n");
};

tempsens::tempsens(pthread_mutex_t* mut)
     :mut_(mut)

{
    printf("Constructor temperature sensor thread class \n");
    temperature.clear();
    rom_vect.clear();
    file_error = 0;
    temps_are_initialized = 0;
};

/// Function seen by POSIX as thread function
void* tempsens::ThreadWrapper(void* data)
{
    // data is a pointer to a Thread, so we can call the
    // real thread function

    (static_cast<tempsens*>(data))->Thread();//Here is where the magic happend to connect this thread object wrapper to the tempsens::Thread(void) funtion below
    return 0;
}

/// This function does the thread work
void tempsens::Thread(void)
{

//check all DS18B20 senors
    printf("DS18B20 thread program started\n");

    char path[50] = ONE_WIRE_DEVICE_PATH;
   // char path[50] = "~/olle";
    printf("%s\n", path);
    char rom[20];
    char buf[100];
    DIR *dirp;
   // QVector<QString> rom_vect;
    struct dirent *direntp;
    int fd =-1;
    char *temp;
    float value;
    // These tow lines mount the device:
    system("sudo modprobe w1-gpio");
    system("sudo modprobe w1-therm");
    // Check if /sys/bus/w1/devices/ exists.
    dirp = opendir(path);
    if(dirp == NULL)
    {
        printf("opendir error\n");
        file_error = 1;
        pthread_mutex_lock(mut_);
        temps_are_initialized = -1;
        pthread_mutex_unlock(mut_);
    }
    if(dirp == NULL){
        printf("Debug1. FAIL open directory ERROR\n");
    }
    else {
        printf("OK open directory\n");
        //************************ Use DS18B20 sensors on Raspberry pi ****************************
        // Reads the directories or files in the current directory.
        direntp = readdir(dirp);
       // int cnt_temp_sens = 0;
        while(direntp != NULL)
        {
            // If 28-00000 is the substring of d_name,
            // then copy d_name to rom and print rom.

            if(!(strstr(direntp->d_name,"w")) && !(strstr(direntp->d_name,".")) && !(strstr(direntp->d_name,"..")))
            {
                strcpy(rom,direntp->d_name);
                printf(" rom: %s\n",rom);
                rom_vect.push_back(QString::fromLocal8Bit(rom));
                temperature.push_back(0.0f);
            }
           // cnt_temp_sens++;
            direntp = readdir(dirp);
        }
        closedir(dirp);
        QByteArray rom_string_char_Array;
        for(int i=0;i<rom_vect.size();i++){
            printf("rom_vector size = %d, i=%d\n", rom_vect.size(), i);
            rom_string_char_Array = rom_vect[i].toLatin1();
            const char *str_rom_string_char;//Prepare a char[] array
            str_rom_string_char = rom_string_char_Array.data();
            printf("str_rom_string_char rom:%s\n", str_rom_string_char);
        }
        // Append the String rom and "/w1_slave" to path
        // path becomes to "/sys/bus/w1/devices/28-00000xxxx/w1_slave"
        strcat(path,rom);
        strcat(path,"/w1_slave");
    //        printf("rom :%s\n", rom);
        // All DS18B20 sensors are set up
        pthread_mutex_lock(mut_);
        temps_are_initialized = 1;
        pthread_mutex_unlock(mut_);

        while(1)
        {
            //Read all the DS18B20 temperature sensors
            for(int i=0;i<rom_vect.size();i++){
                // printf("rom_vector size = %d, i=%d\n", rom_vect.size(), i);
                rom_string_char_Array = rom_vect[i].toLatin1();
                const char *str_rom_string_char;//Prepare a char[] array
                str_rom_string_char = rom_string_char_Array.data();
                char path_l[50] = ONE_WIRE_DEVICE_PATH;
                strcat(path_l,str_rom_string_char);
                strcat(path_l,"/w1_slave");

                // Open the file in the path.

                fd = open(path_l,O_RDONLY);
                if(fd < 0)
                {
                    printf("open error 2\n");
                    file_error = 1;
                }
                // Read the file
                int filesize1 = read(fd,buf,sizeof(buf));

                close(fd);
                if(filesize1 < 0)
                {
                    printf("read error\n");
                    file_error = 1;
                }
                // Returns the first index of 't'.
                temp = strchr(buf,'t');
                // Read the string following "t=".
                sscanf(temp,"t=%s",temp);
                // atof: changes string to float.
                value = (float)(atof(temp))/1000.0f;
                //   printf("rom :%s\n", rom);
                printf(" temp : %3.3f °C",value);
                printf("   sensor nr %d,  ID:%s\n", i+1, str_rom_string_char);


                //*********************************************************
                //********** Read off temperature sensor finnished ********
                //*********************************************************

                printf("Mutex will now lock inside thread and now Update temperature shared memory..\n");
                pthread_mutex_lock(mut_);
                //... Do the update of shared memory here ....
                //This code in between mutex lock should be as short as possible to not block the QT GUI thread more then neccesary just copy over data no others here..
                temperature[i] = value;
                pthread_mutex_unlock(mut_);
                printf("Temperature Thread set to sleep 1ms\n");
                usleep(1000);//Sleep inside this thread.
            }
        }
        //*****************************************************************************************************
    }

}
bool tempsens::Start(void)
{
   // Start the thread, send it the this pointer (points to this class
  // instance)

  //pthread_create() takes 4 arguments.
  //The first argument is a pointer to thread_id which is set by this function.
  //The second argument specifies attributes. If the value is NULL, then default attributes shall be used.
  //The third argument is name of function to be executed for the thread to be created.
  //The fourth argument is used to pass arguments to the function.
  printf("Create and start the temperature sensor thread, this is a separated thread from the QT thread \n");
  return (pthread_create(&threadId_,    // Pointer to the thread handle
                         NULL,          // Optional ptr to thread settings
                         ThreadWrapper, // Thread function
                         this) == 0);   // Argument passed to thread func
}
