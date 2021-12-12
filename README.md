# Lyfco Heatpump HP050-DC-W Raspberry-pi-QT-GUI-with-DS18B20-temperature-sensor-and-PID-controller

https://www.youtube.com/watch?v=0cIs1EkFDYE

### Supported Hardware:

#### Lyfco Heatpump HP050-DC-W
    
#### Raspberry pi

#### DS18B20

## IMPORTANT note

Step 1.

You ALWAYS need to start the python script heatpump socket server FIRST by the command:

    $ python heatpump_server.py
    
Step 2.

Then Start the QT GUI aplication were the socket client is attached

    $ ./housecontrol
    
If you start the QT GUI aplication socket client first then this program will exit automatic becuse no socket server is up.

## QT GUI inteface example (under development)
![](lyfco_GUI.png)

## tinytuya for controlling my Lyfco Heatpump HP050-DC-W

### Install tinytuya

Example of the heatpump_server.py terminal echo

    Received 40 bytes
    Received contents command=6, i1=30, i2=0, i3=0, i=0, i5=0, i6=0, i7=0, i8=0, i9=0
    ON heatpump
    set temperature to 30
    set_status() result {u'devId': u'xxxxxxxxxxxxxxxxxxxxxxxx', u'dps': {u'13': 0, u'109': u'32', u'110': u'15', u'1': True, u'114': u'30', u'3': 38, u'2': 30, u'5':       u'Hot', u'4': u'hot', u'113': u'30', u'112': u'16', u'103': u'on', u'111': u'16', u'106': u'd01', u'107': 32, u'104': u'2', u'105': 94}}
    Send contents back, command=6, i1=30, i2=0, i3=38, i=30, i5=0, i6=555, i7=55, i8=22, i9=0
    Sending it back.. 

    Sent 40 bytes
    
![](GUI_and_heatpump_server2.png)    

Tinytuya don't use Cloud connection to you tuya device.

"tinytuya" connect directly localy to your tuya device (In my case Lyfco Heatpump HP050-DC-W) 

Related links

https://github.com/jasonacox/tinytuya

tinytuya need the LocalKey from you tuya device.

### Extract Tuya LocalKey

Important Note: Don't share your LocalKey, keep your LocalKey secure on your computer.

https://www.youtube.com/watch?v=FpY-xsY-pZ8


## Installation of QT creator on Raspberry pi

### Installing Qt5 Creative:
    $ sudo apt-get update
    $ sudo apt-get install build-essential
    $ sudo apt-get install qtcreator
    $ sudo apt-get install qt5-default
    
### Install BOOST (don't know if nessesary..)
    $ sudo apt install libboost-dev
    $ sudo apt install libboost-all-dev

verify

    $ dpkg -s libboost-dev | grep Version

### Auto start tiny tuya heatpump_server.py

    $ mkdir /home/pi/.config/autostart
    $ nano /home/pi/.config/autostart/clock.desktop
    
``` 
  GNU nano 3.2        /home/pi/.config/autostart/clock.desktop                  

[Desktop Entry]
Type=Application
Name=Clock
Exec=/usr/bin/python /home/pi/tinytuya/heatpump_server.py

``` 

## Auxilary functionality 
    
Control shunt and pumps for
Solar heater
Furnace heater
Pool heat exchanger 
...


### Install wiringPI for GPIO on raspberry pi

    $ sudo apt-get install wiringpi

#### Change LIBS inside QT project file for wiringPI GPIO on raspberry pi

    $ LIBS += -lwiringPi
    
#### Check GPIO pin map 

```
pi@raspberrypi:~ $ gpio readall
 +-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
 |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
 |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
 |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 0 | IN   | TxD     | 15  | 14  |
 |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
 |  17 |   0 | GPIO. 0 |  OUT | 1 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
 |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
 |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
 |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
 |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
 |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
 |  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
 |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
 |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
 |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
 |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
 |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
 |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
 |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
 |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
 +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
 | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
 +-----+-----+---------+------+---+---Pi 3B--+---+------+---------+-----+-----+
pi@raspberrypi:~ $ 
```
    
### Option QWT Ploting

#### Installera QWT Ploting (Optional future develoments with graph plotter)

    $ sudo apt-get install qtdeclarative5-dev-tools
    $ sudo apt-get install libqwt-qt5-dev

#### Change LIBS inside QT project file if using QWT Plotting 

    LIBS += -lqwt-qt5

in the project file for Qt5
with name 

    *.pro 

Example find here:

    https://github.com/berndporr/qwt-example
    
    $ git clone https://github.com/glasgow-bio/qwt-example





 
