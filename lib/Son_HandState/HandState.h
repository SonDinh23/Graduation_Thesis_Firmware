#ifndef HAND_STATE_H
#define HAND_STATE_H

#include <ESP32Servo.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Son_Utils.h>
#include <LinearRegression.h>
#include <Preferences.h>
#include "Adafruit_INA3221.h"

#define LED_PIN   					    4
#define LED_COUNT 					    2

#define IDLE_CONSTANT				    50		//time to enter resting state
#define CURRENT_THRESHOLD 			    200			
#define CURRENT_THRESHOLD_MAX 		    300			
#define CURRENT_THRESHOLD_AVG 		    275	
#define CURRENT_MAX_TEST			    300 //close
#define CURRENT_MIN_TEST			    300 //open

#define MIN_ANGLE_DEFAULT    		    67
#define MAX_ANGLE_DEFAULT    		    101

#define ADDRESS_INA3221_1ST             0x40
#define ADDRESS_INA3221_2ND             0x41

#define COUNT_SERVO                      5

typedef enum {
	STATE_NONE 		= 0,
	STATE_NORMAL 	= 1,
	STATE_OTA 		= 2,
}hand_status_t;

const uint8_t pinServo[COUNT_SERVO] = {32, 33, 25, 26, 27}; // pin for servo

class HandState
{
	public:
        HandState(uint8_t pminAngle = MIN_ANGLE_DEFAULT, uint8_t pmaxAngle = MAX_ANGLE_DEFAULT);
        void begin(void);
        void updateSensor(char pchar);
        void update();

        void onConnect();
        void onDisconnect();

        void setHandStatus(hand_status_t _status);
        void setSpeed(uint8_t pSpeed);
        void setRGB(uint8_t rgbArray[]);

        hand_status_t getHandState();
        uint8_t getSpeedAngle();

    private:
        Preferences pref;

        Servo servoChannel[COUNT_SERVO];

        Adafruit_INA3221 ina3221_1st;
        Adafruit_INA3221 ina3221_2nd;

        Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

        hand_status_t handStatus = STATE_NORMAL;

        byte txAddr[6];     // not use
		byte rxAddr[6];     //Sensor box address

        int8_t angleOpen = -1;								//gradually go to large angle
		int8_t angleClose = 1;								//gradually go to small angle

        uint8_t speed = 1;
		float openSpeed = 1.0;             		// multiply this per step
		float closeSpeed = 1.0;             	// multiply this per step 
		uint8_t startAngle = 90;

        uint8_t angle = startAngle;         	// when turn on, hand goes to this angle
		uint16_t minAngle = 80;            		// servo lower limit
		uint16_t maxAngle = 100;  
        
        float pwmValue;
        uint16_t pwmValueMin;
		uint16_t pwmValueMax;// servo upper limit

        uint8_t RGB[3] = {0};					
		bool isConnect = false;							//state BLE

        void setServo();

        void readSpeed();
};

#endif