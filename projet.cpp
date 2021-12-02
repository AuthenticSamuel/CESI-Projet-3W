#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <BME280I2C.h>
#include <Wire.h>
#include <stdio.h>
#include <stdlib.h>
#include <SoftwareSerial.h>
// #include <avr8-stub.h>

#define LEDR 9
#define LEDG 8
#define LEDB 7
#define BUTTONR 2
#define BUTTONG 3
#define INTERRUPTG 3
#define INTERRUPTR 2
#define BAUDRATE 9600



SoftwareSerial SoftSerial(2,3);
unsigned char buffer[64];
int count=0;  
long time;

typedef struct {

	unsigned int LOG_INTERVAL = 10; // 600 pour 10 minutes
	unsigned short SIZE = 4096;
	unsigned short TIMEOUT = 10;
	bool LUMIN = 1;
	int LUMIN_LOW = 255;
	int LUMIN_HIGH = 768;
	bool TEMP_AIR = 1;
	int MIN_TEMP_AIR = -10;
	int MAX_TEMP_AIR = 60;
	bool HYGR = 1;
	int HYGR_MINT = 0;
	int HYGR_MAXT = 50;
	bool PRESSURE = 1;
	int PRESSURE_MIN = 850;
	int PRESSURE_MAX = 1080;

} variables;

variables current;

typedef struct {

	float humidity;
	float pressure;
	float temp;
	int light;

} capteurs;

capteurs values;

File myFile;
BME280I2C bme;
RTC_DS1307 rtc;

byte mode = 3;
// 1 Standard
// 2 Economique
// 3 Maintenance
// 4 Configuration



void outputLed(byte red, byte green, byte blue);
void inputRTC();
void inputTemperature();
void inputPression();
void inputHumidity();
void inputLuminosite();
void inputGPS();
void inputButtons();

// Check
void check() {

	byte timeoutDeath = 0;
	while (!rtc.begin()) {
		
		if (timeoutDeath >= current.TIMEOUT) {

			Serial.println(F("Could not find RTC."));
			while (1);

		}

		outputLed(255, 0, 0);
		delay(500);
		outputLed(0, 0, 255);
		delay(500);
		timeoutDeath++;
	
	}

	timeoutDeath = 0;
	while (!bme.begin()) {
		
		if (timeoutDeath >= current.TIMEOUT) {

			Serial.println(F("Could not find BME."));
			while (1);

		}

		outputLed(255, 0, 0);
		delay(500);
		outputLed(0, 255, 0);
		delay(500);
		timeoutDeath++;
	
	}

	timeoutDeath = 0;
	while (!SD.begin(4)) {

		if (timeoutDeath >= current.TIMEOUT) {

			Serial.println(F("Could not find SD card"));
			while (1);
		
		}

		outputLed(255, 0, 0);
		delay(333);
		outputLed(255, 255, 255);
		delay(667);
		timeoutDeath++;

	}

	// timeoutDeath = 0;
	// while (!GPS.begin()) {

	// 	if (timeoutDeath >= current.TIMEOUT) {
			
	// 		Serial.println(F("Could not find GPS"));
	// 		while (1);
			
	// 	}

	// 	outputLed(255, 0, 0);
	// 	delay(500);
	// 	outputLed(255, 165, 0);
	// 	delay(500);
	// 	timeoutDeath++;
		
	// }

}


// Modes
void modeStandard() {

	outputLed(0, 255, 0); 	// Vert
	inputRTC();
	inputHumidity();
	inputPression();
	inputTemperature();
	inputLuminosite();
	// inputGPS();

}

void modeEco() {

	outputLed(0, 0, 255);	// Bleu
	inputRTC();
	inputHumidity();
	inputPression();
	inputTemperature();
	inputLuminosite();

}

void modeMaintenance() {

	outputLed(255, 30, 0);	// Orange
	inputRTC();
	inputHumidity();
	inputPression();
	inputTemperature();
	inputLuminosite();
	inputGPS();

	Serial.print("Humidity: ");
	Serial.println(values.humidity);
	Serial.print("Temperature: ");
	Serial.println(values.temp);
	Serial.print("Pressure: ");
	Serial.println(values.pressure);
	Serial.println("GPS: N/A");
	Serial.print("Luminosity: ");
	Serial.println(values.light);

}

void CHANGE_PARAMS() {
	
	String read = "";
	int command;
	Serial.println(F("Input the param you want to change, and put. Choices : \n1) LUMIN\n2) LUMIN_LOW\n LUMIN_HIGH "));
	while (!Serial.available()) delay(500);
	read = Serial.readString();
	command = read.toInt();
	switch (command) {
		case 1:
			Serial.println(F("LUMIN. Choices : \n1) True\n2) False"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			if(read == "1") current.LUMIN = 1;
			else if (read == "2") current.LUMIN = 0;
			break;

		case 2:
			Serial.println(F("LUMIN_LOW"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.LUMIN_LOW = read.toInt();
			break;
		case 3:
			Serial.println(F("LUMIN_HIGH"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.LUMIN_HIGH = read.toInt();
			break;
		case 4:
			Serial.println(F("TEMP_AIR . Choices : \n1) True\n2) False"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			if(read == "1") current.LUMIN = 1;
			else if (read == "2") current.LUMIN = 0;
			break;
		case 5:
			Serial.println(F("MIN_TEMP_AIR"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.MIN_TEMP_AIR = read.toInt();
			break;
		case 6:
			Serial.println(F("MAX_TEMP_AIR"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.MAX_TEMP_AIR = read.toInt();
			break;
		case 7:
			Serial.println(F("HYGR. Choices : \n1) True\n2) False"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			if(read == "1") current.HYGR = 1;
			else if (read == "2") current.HYGR = 0;
			break;
		case 8:
			Serial.println(F("HYGR_MINT"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.HYGR_MINT = read.toInt();
			break;
		case 9: 
			Serial.println(F("HYGR_MAXT"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.HYGR_MAXT = read.toInt();
			break;
		case 10:
			Serial.println(F("PRESSURE: \n1) True\n2) False"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			if(read == "1") current.PRESSURE = 1;
			else if (read == "2") current.PRESSURE = 0;
			break;
		case 11:
			Serial.println(F("PRESSURE_MIN"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.PRESSURE_MIN = read.toInt();
			break;
		case 12:
			Serial.println(F("PRESSURE_MAX"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.PRESSURE_MAX = read.toInt();
			break;
	}
}

void modeConfig() {

	outputLed(255, 255, 0);	// Jaune
	variables defaut;
	defaut.LOG_INTERVAL = 10;
	defaut.TIMEOUT = 1;
	defaut.SIZE = 256;
	current.LOG_INTERVAL = 4000;
	Serial.println(F("Input you command: "));
	Serial.println(F("Command detected you have the choice between : \n-1) CHANGE_INTERVAL \n-2) CHANGE_SIZE \n-3) RESET"));
	unsigned long time = millis();

	while (!Serial.available() && millis() - time < 30000) delay(500);
	
	if (millis() - time > 30000) {

		Serial.println(F("Activity time reached")); 
		return;
	
	} 

	String read = "";
	String read2 = "";
	short result;
	short command;
	read2 = Serial.readString();
	command = read2.toInt();
	switch (command) {
		
		case 1:
			Serial.println(F("Input your new interval (in second)"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			result = read.toInt();
			current.LOG_INTERVAL = result;
			Serial.print(F("Done, your new interval is set to:"));
			Serial.println(current.LOG_INTERVAL);
			break;

		case 2:
			Serial.println(F("Input your new max size"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			result = read.toInt();
			Serial.println(read2);
			break;
		
		case 3:
			Serial.println(F("Are you sure you want to reset ? Y/N"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			if(read == "Y"){
				current.LOG_INTERVAL = defaut.LOG_INTERVAL;
				current.SIZE = defaut.SIZE;
				current.TIMEOUT = defaut.TIMEOUT;
				current.LUMIN = defaut.LUMIN;
				current.LUMIN_LOW = defaut.LUMIN_LOW;
				current.LUMIN_HIGH = defaut.LUMIN_HIGH;
				current.TEMP_AIR = defaut.TEMP_AIR;
				current.MIN_TEMP_AIR = defaut.MIN_TEMP_AIR;
				current.MAX_TEMP_AIR = defaut.MAX_TEMP_AIR;
				current.HYGR = defaut.HYGR;
				current.HYGR_MINT = defaut.HYGR_MINT;
				current.HYGR_MAXT = defaut.HYGR_MAXT;
				current.PRESSURE = defaut.PRESSURE;
				current.PRESSURE_MIN = defaut.PRESSURE_MIN;
				current.PRESSURE_MAX = defaut.PRESSURE_MAX;
				Serial.println(F("Default values restored"));
				Serial.println(current.LOG_INTERVAL);
			} 
			else Serial.println(F("Cancelled..."));
			break;
		case 4: //check version 
			Serial.println(F("Input your new timeout"));
			while (!Serial.available()) delay(500);
			read = Serial.readString();
			current.TIMEOUT = read.toInt();
			break;

		case 5:
			CHANGE_PARAMS();
			break;

		case 6: //change date format
			Serial.println(F("Input the second time"));
			while (!Serial.available()) delay(500);
			String secondH = Serial.readString();
			Serial.println(F("Input the minute time"));
			while (!Serial.available()) delay(500);
			String minuteH = Serial.readString();
			Serial.println(F("Input the hour time"));
			while (!Serial.available()) delay(500);
			String hourH = Serial.readString();
			Serial.println(F("Input the day"));
			while (!Serial.available()) delay(500);
			String dayH = Serial.readString();
			Serial.println(F("Input the month"));
			while (!Serial.available()) delay(500);
			String monthH = Serial.readString();
			Serial.println(F("Input the year"));
			while (!Serial.available()) delay(500);
			String yearH = Serial.readString();
			rtc.adjust(DateTime(yearH.toInt(), monthH.toInt(), dayH.toInt(), hourH.toInt(), minuteH.toInt(), secondH.toInt())); //January 21, 2014 at 3am:
			break;

		default:
			Serial.println(F("Invalid command try again"));

	}
}





// Inputs
void inputRTC() {

	DateTime now = rtc.now();
	// Serial.println(now);
	String timestamp = "";
	timestamp += now.day();
	timestamp += "/";
	timestamp += now.month();
	timestamp += "/";
	timestamp += now.year();
	timestamp += " - ";
	timestamp += now.hour();
	timestamp += ":";
	timestamp += now.minute();
	timestamp += ":";
	timestamp += now.second();
	// Serial.println(timestamp);

}

void inputGPS() {
	if (SoftSerial.available()) {
		while(SoftSerial.available()) {

            buffer[count++]=SoftSerial.read();   
            if(count == 64)break;

        }

        Serial.write(buffer,count);              
        for (int i=0; i<count;i++) buffer[i]=NULL;                               
        count = 0;
		
	}
}
   
	

void inputHumidity() {
	
	if(current.HYGR) {

		float temp(NAN), hum(NAN), pres;
		bme.read(pres, temp, hum);
		values.humidity = hum;

	}
	
}

void inputTemperature() {
	
	if(current.TEMP_AIR) {
		
		float temp, hum, pres;
		BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
		bme.read(pres, temp, hum);
		values.temp = temp;
	
	}
	
}

void inputPression() {
	
	if(current.PRESSURE) {
		
		float temp(NAN), hum(NAN), pres(NAN);
		bme.read(pres, temp, hum);
		values.pressure = pres;
		
	}
	
}

void inputLuminosite() {
	
	if (current.LUMIN) {
		
		short lux = analogRead(A0);
		values.light = lux;

	}
	
}


void inputButtonInterruptR() {

	Serial.println("button pressed");
	if(!digitalRead(INTERRUPTR)) time = millis();
	else if(digitalRead(INTERRUPTR)) {

		if(millis()-time >= 5000) {

			if(mode == 1 || mode == 2) mode = 3;
			else if(mode == 3) mode = 1;

		} else return;
	}
	
	Serial.print("current mode :");
	Serial.println(mode);
	
}

void inputButtonInterruptG() {

	Serial.println("button pressed");
	if(!digitalRead(INTERRUPTG)) time = millis();
	else if(digitalRead(INTERRUPTG)) {
		
		if(millis()-time >= 5000) {

			if(mode == 1) mode = 2;

			else if(mode == 2) mode = 1;

		}
		
		else return;

	}
	Serial.print("current mode :");
	Serial.println(mode);

}


// Outputs
void outputLed(byte red, byte green, byte blue) {

    analogWrite(LEDR, red);
    analogWrite(LEDG, green);
    analogWrite(LEDB, blue);

}

void outputSD() {

	

}

void outputSerial() {



}

// Garder a la fin du code
void setup() {

	// debug_init();
	// DateTime now = rtc.now();
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
	pinMode(BUTTONR, INPUT_PULLUP);
	pinMode(BUTTONG, INPUT_PULLUP);
	Serial.begin(BAUDRATE);
	Wire.begin();
	SoftSerial.begin(BAUDRATE);
	check();

	if (!digitalRead(BUTTONR)) {

		mode = 4;
		loop();
		
	}

	// attachInterrupt(digitalPinToInterrupt(INTERRUPTR), inputButtonInterruptR, CHANGE);
	// attachInterrupt(digitalPinToInterrupt(INTERRUPTG), inputButtonInterruptG, CHANGE);

}

void loop() {
	switch (mode) {

		case 4:
			modeConfig();
			break;

		case 3:
			modeMaintenance();
			break;
		
		case 2:
			modeEco();
			current.LOG_INTERVAL *= 2;
			break;
		
		case 1:
		default:
			modeStandard();
			break;

	}

	char* filename = "testchar.txt";

	myFile = SD.open(filename, FILE_WRITE);

	if (myFile) {

		if (mode == 3) Serial.print("Writing to finalfinal.txt...");
		myFile.print("temp:");
		myFile.print(values.temp);
		myFile.print("   hum :");
		myFile.print(values.humidity);
		myFile.print("   pressure :");
		myFile.print(values.pressure);
		myFile.print("   light :");
		myFile.println(values.light);
		myFile.close();

		if (mode == 3) Serial.println("Done.");
		
	}

	myFile = SD.open(filename);
	if (myFile) {

		if (mode == 3) {

			Serial.println("finalfinal.txt: ");
			while (myFile.available()) Serial.write(myFile.read());

		}

		myFile.close();

	} else Serial.println("Error opening finalfinal.txt.");
	
	Serial.print(F("\n"));
	delay(current.LOG_INTERVAL * 1000);

}
