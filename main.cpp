#include "mbed.h"
#include "Servo.h"
#include "motordriver.h"
#include "ultrasonic.h"
#include <stdio.h>
Serial pc(USBTX,USBRX);
Serial blue(p13,p14);
DigitalOut myled(LED1);      


#define AUTOPILOT 10
#define FORWARD 1
#define REVERSE -1
#define STOP 0

int state = 0; //global variable stop state 
int status1;
int status2;
int autoPilotLock;
int distanceCenter;
int distanceLeft;
int distanceRight;
Motor M(p21, p22, p23, 1); // pwm, fwd, rev, can brake 
Servo S(p24);


void getNewState() {
    //Logic for AdaFruit App
    char bnum=0;
    char bhit=0;
    if (blue.readable()) {
    if (blue.getc()=='!') {
        if (blue.getc()=='B') { //button data packet
            bnum = blue.getc(); //button number
            bhit = blue.getc(); //1=hit, 0=release
            if (blue.getc()==char(~('!' + 'B' + bnum + bhit))) { //checksum OK?
                myled = bnum - '0'; //current button number will appear on LEDs
                switch (bnum) {
                    case '1': //AutoPilot Mode
                        if (bhit=='1') {
                            state = 10; //autopilot state
                        }
                        break;
                    case '5': //forward
                        if (bhit=='1') {
                            state = 1; //forward state
                        } else {
                            state = 0; //stop state
                        }
                        break;
                    case '6': //reverse
                        if (bhit=='1') {
                            state = -1; //reverse state
                        } else {
                            state = 0; //stop state
                        }
                        break;
                    case '7': //left
                        if (bhit=='1') {
                            S = S + 0.5; //turn left                                
                        }
                        break;
                    case '8': //right
                        if (bhit=='1') {
                            S = S - 0.5; //turn right
                        }
                        break;
                    }
                }
            }
        }
    }
}

void autoPilot() {
    autoPilotLock = 0;
    if (distanceCenter >= 200 && autoPilotLock == 0) {
        M.speed(1.0);
        S = 0.5;
    }
    else if (distanceRight > distanceLeft) { //More space on right so turn right
        autoPilotLock = 1;
        M.speed(-1.0);
        S = 0;
        wait_ms(1000);
        autoPilotLock = 0;
    }
    else { //or turn left
        autoPilotLock = 1;
        M.speed(-1.0);
        S = 1.0;
        wait_ms(1000);
        autoPilotLock = 0;
    }
}

void dist1(int distance)
{
    distanceCenter = distance;
}

void dist2(int distance)
{
    distanceLeft = distance;
}

void dist3(int distance)
{
    distanceRight = distance;
}

ultrasonic muCenter(p6, p7, .1, 1, &dist1); //sonar 1 initialization
ultrasonic muLeft(p17, p18, .1, 1, &dist2); //sonar 2 initialization
ultrasonic muRight(p15, p16, .1, 1, &dist3); //sonar 3 initialization



int main() {
    //SONAR Initializations
    muCenter.startUpdates();//SONAR 1 starts measuring the distance 
    muLeft.startUpdates();//SONAR 2 starts measuring the distance 
    muRight.startUpdates();//SONAR 3 starts measuring the distance 
    while(1) { //main loop for motor control
        pc.printf("Center D=%ld mm\r\n", distanceCenter);
        pc.printf("Right D=%ld mm\r\n", distanceRight);
        pc.printf("Left D=%ld mm\r\n", distanceLeft);
        muCenter.checkDistance();  //SONAR measuring starts
        muLeft.checkDistance();
        muRight.checkDistance();
        getNewState();
        if (distanceCenter >= 250 && state == FORWARD) {
            M.speed(1.0);
        }
        else if (state == REVERSE) {
            M.speed(-1.0);
        }
        else if (state == AUTOPILOT) {
            autoPilot();
        }
        else {
            M.speed(0);
        }
    }
    
}
