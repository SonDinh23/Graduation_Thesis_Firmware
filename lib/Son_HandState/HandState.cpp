#include "HandState.h"

HandState::HandState(uint8_t pminAngle, uint8_t pmaxAngle):
	minAngle(pminAngle),
	maxAngle(pmaxAngle) {
}

void HandState::begin(void) {
    angleOpen = -1;
	angleClose = 1;

    if (!ina3221_1st.begin(ADDRESS_INA3221_1ST, &Wire)) {
        Serial.println("Failed to find ina3221 1st chip");
    }

    if (!ina3221_2nd.begin(ADDRESS_INA3221_2ND, &Wire)) {
        Serial.println("Failed to find ina3221 2nd 1st chip");
    }

    // Set shunt resistances for all channels to 0.1 ohms
    for (uint8_t i = 0; i < 3; i++) {
        ina3221_1st.setShuntResistance(i, 0.1);
        ina3221_2nd.setShuntResistance(i, 0.1);
    }

    // Set a power valid alert to tell us if ALL channels are between the two limits:
    ina3221_1st.setPowerValidLimits(3.0, 15.0);
    ina3221_2nd.setPowerValidLimits(3.0, 15.0);

    pref.begin("hand-info", false);

    ESP32PWM::allocateTimer(0);
  	ESP32PWM::allocateTimer(1);
  	ESP32PWM::allocateTimer(2);
  	ESP32PWM::allocateTimer(3);

    // set up servo channels
    for (uint8_t i = 0; i < 5; i++) servoChannel[i].attach(pinServo[i]);
    delay(500);

    minAngle = pref.getUShort("minAngle", MIN_ANGLE_DEFAULT);
	maxAngle = pref.getUShort("maxAngle", MAX_ANGLE_DEFAULT);

    Serial.printf("MinAngle: %d\n", minAngle);
	Serial.printf("MaxAngle: %d\n", maxAngle);

	startAngle = (minAngle + maxAngle)/2;
    angle = startAngle;
	Serial.printf("startAngle: %d\n", startAngle);
	for (uint8_t i = 0; i < 5; i++) servoChannel[i].write(angle);
    pwmValueMin = map(minAngle, 0, 180, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
	pwmValueMax = map(maxAngle, 0, 180, DEFAULT_uS_LOW, DEFAULT_uS_HIGH);
	Serial.printf("pwmValueMin: %d\n", pwmValueMin);
	Serial.printf("pwmValueMax: %d\n", pwmValueMax);

    readSpeed();

    pixels.begin();
	pixels.setBrightness(10);
	pixels.clear();
	pixels.show();
}

void HandState::onConnect() {
	isConnect = true;
	pixels.setPixelColor(1, pixels.Color(0, 0, 255));
	pixels.show();
}

void HandState::onDisconnect() {
	isConnect = false;
	pixels.setPixelColor(1, pixels.Color(255, 0, 0));
	pixels.show();
}

void HandState::setHandStatus(hand_status_t _status) {
	Serial.printf("setHandStatus: %d\n", _status);
	handStatus = _status;
}

void HandState::setSpeed(uint8_t _speed) {
	speed = _speed;
	switch (speed) {
		case 1:
			openSpeed = 0.75; 
			closeSpeed = 0.75;
			break;

		case 2:
			openSpeed = 1; 
			closeSpeed = 1;
			break;

		case 3:
			openSpeed = 1.25; 
			closeSpeed = 1.25;
			break;
		
		default:
			openSpeed = 0.75; 
			closeSpeed = 0.75;
			break;
	}

	pref.putUShort("speedAngle", speed);
	Serial.printf("SpeedAngle: %d\n",speed);
}

void HandState::setRGB(uint8_t rgbArray[]) {
	for (uint8_t i = 0; i < 3; i++) {
		RGB[i] = rgbArray[i];
	}
	pixels.setPixelColor(0, pixels.Color(RGB[0], RGB[1], RGB[2]));
	pixels.show();
	// Serial.printf("RGB: %d:%d:%d\n", RGB[0], RGB[1], RGB[2]);
}

hand_status_t HandState::getHandState() {
	return handStatus;
}

uint8_t HandState::getSpeedAngle() {
	return speed;
}

void  HandState::readSpeed() {
	speed = pref.getUShort("speedAngle", 1);
	Serial.printf("SpeedAngle: %d\n",speed);
	switch (speed) {
		case 1:
			openSpeed = 0.75; 
			closeSpeed = 0.75;
			break;

		case 2:
			openSpeed = 1; 
			closeSpeed = 1;
			break;

		case 3:
			openSpeed = 1.25; 
			closeSpeed = 1.25;
			break;
		
		default:
			openSpeed = 0.75; 
			closeSpeed = 0.75;
			break;
	}
}

void HandState::updateSensor(char pchar) {
	
}

void HandState::update() {
	setServo();
}

void HandState::setServo() {

}