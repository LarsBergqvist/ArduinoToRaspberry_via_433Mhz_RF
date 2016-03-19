// Depends on the RCSwitch library https://github.com/sui77/rc-switch
// and the DHT sensor-, OneWire- and DallasTemperature libraries

#include "RCSwitch.h"
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Unique ID:s (4 bits, 0-15) for each measurement type so that the receiver
// understands how to interpret the data on arrival
#define LDR_MEASUREMENT_ID      1
#define TEMP_MEASUREMENT_ID     2
#define HUMIDITY_MEASUREMENT_ID 3
#define OUTDOOR_MEASUREMENT     4

// Setup for light sensor
#define LIGHT_IN 0                    // Light measurement goes to A0 pin

// Setup for DHT11
#define DHTPIN 2                      // Signal in from DHT11 goes to digital pin 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Setup for DS18B20
#define ONE_WIRE_BUS 3                // Uses digital pin 3
OneWire ourWire(ONE_WIRE_BUS);
DallasTemperature sensors(&ourWire);

// Setup for radio transmitter
#define TX_PIN 10                     // PWM output pin to use for transmission
#define DELAY_BETWEEN_TRANSMITS 60000 // in milliseconds
RCSwitch transmitter = RCSwitch();
 
void setup() 
{  
  Serial.begin(9600);

  pinMode(LIGHT_IN, INPUT);
     
  transmitter.enableTransmit(TX_PIN); 
  transmitter.setRepeatTransmit(25);
}

unsigned long Code32BitsToSend(int measurementTypeID, unsigned long seq, unsigned long data)
{
    unsigned long checkSum = measurementTypeID + seq + data;
    unsigned long byte3 = ((0x0F & measurementTypeID) << 4) + (0x0F & seq);
    unsigned long byte2_and_byte_1 = 0xFFFF & data;
    unsigned long byte0 = 0xFF & checkSum;
    unsigned long dataToSend = (byte3 << 24) + (byte2_and_byte_1 << 8) + byte0;

    return dataToSend;
}

// Encode a float as two bytes by multiplying with 100
// and reserving the highest bit as a sign flag
// Values that can be encoded correctly are between -327,67 and +327,67
unsigned int EncodeFloatToTwoBytes(float floatValue)
{
  bool sign = false;
  
  if (floatValue < 0) 
    sign=true;  
      
  int integer = (100*fabs(floatValue));
  unsigned int word = integer & 0XFFFF;
  
  if (sign)
    word |= 1 << 15;

  return word;
}

// A rolling sequence number for each measurement
// Restarts at 0 after seqNum=15 has been used
unsigned long seqNum=0;
unsigned long previousTime = 0; 
void loop() 
{
    unsigned long currentTime = millis();
    if (currentTime - previousTime <= DELAY_BETWEEN_TRANSMITS)
    {
      return;
    }

    previousTime = currentTime;

    //
    // DS18B20 sensor, outdoor temperature
    //
    sensors.requestTemperatures();
    float outdoorTemp = sensors.getTempCByIndex(0);
    unsigned int encodedFloat = EncodeFloatToTwoBytes(outdoorTemp);

    unsigned long dataToSend = Code32BitsToSend(OUTDOOR_MEASUREMENT,seqNum,encodedFloat);
    // Send 32 bits of data
    transmitter.send(dataToSend, 32);

    delay(2000);
    
    //
    // Get the light measurement (0-1023)
    //
    unsigned int data = analogRead(LIGHT_IN);
    dataToSend = Code32BitsToSend(LDR_MEASUREMENT_ID,seqNum,data);
    // Send 32 bits of data
    transmitter.send(dataToSend, 32);

    delay(2000);

    //
    // DHT11, get temperature
    //
    unsigned long t = (unsigned long)dht.readTemperature();
    if (!isnan(t)) 
    {
      dataToSend = Code32BitsToSend(TEMP_MEASUREMENT_ID,seqNum,t);
      transmitter.send(dataToSend, 32);
    }

    delay(2000);

    //
    // DHT11, get humidity
    //
    unsigned long h = (unsigned long)dht.readHumidity();
    if (!isnan(h)) 
    {
      dataToSend = Code32BitsToSend(HUMIDITY_MEASUREMENT_ID,seqNum,h);
      transmitter.send(dataToSend, 32);
    }
      
    seqNum++;
    if (seqNum > 15)
    {
      seqNum = 0;
    }    
}

