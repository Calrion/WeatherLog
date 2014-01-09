/*
 * WeatherLog.ino
 * WeatherLog
 * Created by Greg Waterhouse, 2014-01-05.
 *
 *
 * (c) Copyright 2014 Greg Waterhouse.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ---
 *
 * Based on code created 2008, updated 2010 by Tom Igoe, which is in the 
 * public domain.
 *
 */

#include <math.h>
#include <Time.h>
#include <SoftwareSerial.h>
#include <TinyGPS.h>

#define APP_NAME "WeatherLog"
#define APP_VERSION "1.7.0"

#define SUPPLY_VOLTS 5.0
#define ADC_PRECISION 10

#define PIN_SENSOR_TEMP A0
#define PIN_SENSOR_RH A1
#define PIN_GPS_RX 3 // Arduino pin wired to GPS RX pin (TX on Arduino).
#define PIN_GPS_TX 2 // Arduino pin wired to GPS TX pin (RX on Arduino).
#define PIN_GPS_PPS 10 // Pulses high for 100ms every 1s. Trigger rising edge.
#define PIN_GPS_5V0 A5 // Set high to provide 5V DC supply to GPS (to 40mA).

#define SERIAL_BAUD 9600 // Baud rate of USB serial connection to PC.
#define SERIAL_BAUD_GPS 9600
#define SERIAL_DELTA 30000 // Time between serial data output in ms.
#define SERIAL_FLOAT_PRECISION 2 // Decimal places to display for float data.

// Pre-calculated smoothing factors
#define SMOOTH_FACTOR 0.0147783251231526 // 1 - (1 / 1.015)
#define ONE_MINUS_SMOOTH 0.985221674876847 // 1 - PC_SMOOTH_FACTOR
#define smooth(value, newValue) ((value) == 0) ? (newValue) : (((newValue) * SMOOTH_FACTOR) + ((value) * ONE_MINUS_SMOOTH))

typedef struct _sensorReading {
	float value;
	float min;
	float max;
	double sum;
	int count;
};
#define SensorReading struct _sensorReading

const struct _sensorReading SensorReadingZero = {0, 0, 0, 0, 0};

#define sensorReadAverage(sensor) ((sensor)->sum / (sensor)->count)
#define sensorReadValue(sensor) ((sensor)->value)

void sensorWriteValue(struct _sensorReading *sensor, float value) {
	sensor->value = value;
	sensor->sum += value;
	sensor->count++;
	sensor->min = ((sensor->value < sensor->min) || (sensor->min == 0)) ? sensor->value : sensor->min;
	sensor->max = (sensor->value > sensor->max) ? sensor->value : sensor->max;
	// sensorDebug(sensor);
}

void sensorDebug(struct _sensorReading *sensor) {
	Serial.print("*** ");
	Serial.print(sensor->value, DEC);
	Serial.print(", ");
	Serial.print(sensor->min, DEC);
	Serial.print(", ");
	Serial.print(sensor->max, DEC);
	Serial.print(", ");
	Serial.print(sensor->sum, DEC);
	Serial.print(", ");
	Serial.print(sensor->count, DEC);
	Serial.print(" ***\r\n");
}

// Wrap analogRead() to provide offset- and slope-based value, not count
// Slope is assumed to be linear!
float sensorRead(int analogPin, float offsetVolts, float slopeVolts) {
	float voltage = 0;
	
	voltage = analogRead(analogPin) * (SUPPLY_VOLTS / pow(2, ADC_PRECISION));
	voltage = ((voltage - offsetVolts) * (1 / slopeVolts));

	return voltage;
}

float getTemp(int analogPin) {
	return sensorRead(analogPin, 0.5, 0.01);
}

// Warning: this uses raw sensor values; the temperature 
// compensation may be less accurate than desirable.
float getRH(int analogPinRH, int analogPinTemp) {
	float rhValue = sensorRead(analogPinRH, 0.958, 0.03068);
	
	return rhValue / (1.0546 - (0.00216 * getTemp(analogPinTemp)));
}


// Setup serial GPS
SoftwareSerial gpsSerial(PIN_GPS_TX, PIN_GPS_RX);
TinyGPS gps;


void processTemperature(struct _sensorReading *temp) {
	sensorWriteValue(temp, getTemp(PIN_SENSOR_TEMP));
}

void processRelativeHumidity(struct _sensorReading *relhum) {
	sensorWriteValue(relhum, getRH(PIN_SENSOR_RH, PIN_SENSOR_TEMP));
}

void writeSerialData(unsigned long *lastUpdate, struct _sensorReading *temp, struct _sensorReading *relhum) {
	char buffer[79]; // A 78-char line + \0
	float temperature;
	float relativeHumidity;
	
	if((*lastUpdate + SERIAL_DELTA) < millis()) {
		unsigned long gps_date, gps_time, gps_fixage;
		
		gps.get_datetime(&gps_date, &gps_time, &gps_fixage);
		temperature = sensorReadAverage(temp);
		relativeHumidity = sensorReadAverage(relhum);
		*lastUpdate += SERIAL_DELTA;

		sprintf(buffer, "%06ld %08ld,%ld,", gps_date, gps_time, gps_fixage);
		Serial.print(buffer);
		Serial.print(temperature, SERIAL_FLOAT_PRECISION);
		Serial.print(",");
		Serial.print(relativeHumidity, SERIAL_FLOAT_PRECISION);
		Serial.print("\r\n");
		
		*temp = SensorReadingZero;
		*relhum = SensorReadingZero;
	}
}

void processGPS() {
	while(gpsSerial.available()) {
		char c = gpsSerial.read();
		gps.encode(c);
	}
}

void setup() {
	char buffer[79]; // A 78-char line + \0

	Serial.begin(SERIAL_BAUD);
	
	// Setup GPS
	pinMode(PIN_GPS_5V0, OUTPUT);
	digitalWrite(PIN_GPS_5V0, HIGH);
	gpsSerial.begin(SERIAL_BAUD_GPS);
	delay(3000);
	for (unsigned long start = millis(); millis() - start < 1000;) {
		// Grab a few NMEA sentences to seed the GPS
		while(gpsSerial.available()) { gps.encode(gpsSerial.read()); }
	}
	
	// Setup serial communications
	sprintf(buffer, "\r\n\r\n%s v%s\r\n%s\r\n", APP_NAME, APP_VERSION, "date_time,fix_age,temperature,relative_humidity");
	Serial.print(buffer);
}

void loop() {
	static unsigned long lastUpdate = 0;
	static SensorReading temperature = SensorReadingZero;
	static SensorReading relativeHumidity = SensorReadingZero;
	
	processGPS();
	processTemperature(&temperature);
	processRelativeHumidity(&relativeHumidity);

	writeSerialData(&lastUpdate, &temperature, &relativeHumidity);
}

