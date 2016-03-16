// Depends on the RCSwitch library https://github.com/sui77/rc-switch
// and the DHT sensor library

#include "RCSwitch.h"
#include <DHT.h>

#define LIGHT_IN 0                    // Light measurement goes to A0 pin
#define DHTPIN 2                      // Signal in from DHT11 goes to digital pin 2
#define DHTTYPE DHT11

// Unique ID:s (4 bits, 0-15) for each measurement type so that the receiver
// understands how to interpret the data on arrival
#define LDR_MEASUREMENT_ID 1
#define TEMP_MEASUREMENT_ID 2
#define HUMIDITY_MEASUREMENT_ID 3

#define TX_PIN 10                     // PWM output pin to use for transmission
#define DELAY_BETWEEN_TRANSMITS 5000  // in milliseconds

DHT dht(DHTPIN, DHTTYPE);

RCSwitch transmitter = RCSwitch();
 
void setup() 
{  
  Serial.begin(9600);

  pinMode(LIGHT_IN, INPUT);
     
  transmitter.enableTransmit(TX_PIN); 
  transmitter.setRepeatTransmit(25);
}

// A rolling sequence number for each measurement
// Restarts at 0 after seqNum=15 has been used
unsigned long seqNum=0;

unsigned long Code32BitsToSend(int measurementTypeID, unsigned long seq, unsigned long data)
{
    unsigned long checkSum = measurementTypeID + seq + data;
    unsigned long byte3 = ((0x0F & measurementTypeID) << 4) + (0x0F & seq);
    unsigned long byte2_and_byte_1 = 0xFFFF & data;
    unsigned long byte0 = 0xFF & checkSum;
    unsigned long dataToSend = (byte3 << 24) + (byte2_and_byte_1 << 8) + byte0;

    return dataToSend;
}

unsigned long previousTime = 0; 
void loop() 
{
    unsigned long currentTime = millis();
    if (currentTime - previousTime <= DELAY_BETWEEN_TRANSMITS)
    {
      return;
    }

    previousTime = currentTime;
    
    // Get the light measurement (0-1023)
    unsigned int data = analogRead(LIGHT_IN);
    unsigned long dataToSend = Code32BitsToSend(LDR_MEASUREMENT_ID,seqNum,data);
    // Send 32 bits of data
    transmitter.send(dataToSend, 32);

    delay(2000);
    
    unsigned long t = (unsigned long)dht.readTemperature();
    dataToSend = Code32BitsToSend(TEMP_MEASUREMENT_ID,seqNum,t);
    if (!isnan(t)) 
    {
      transmitter.send(dataToSend, 32);
    }

    delay(2000);

    unsigned long h = (unsigned long)dht.readHumidity();
    dataToSend = Code32BitsToSend(HUMIDITY_MEASUREMENT_ID,seqNum,h);
    if (!isnan(h)) 
    {
      transmitter.send(dataToSend, 32);
    }
      
    seqNum++;
    if (seqNum > 15)
    {
      seqNum = 0;
    }    
}

