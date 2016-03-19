# Python 2.7.9
# Uses pi_switch from https://github.com/lexruee/pi-switch-python
# See pi_switch readme for details on setup

from pi_switch import RCSwitchReceiver
import time
import csv

receiver = RCSwitchReceiver()
receiver.enableReceive(2)

acceptedTypes = { 1 : "Indoor Light", 2 : "Indoor Temp", 3: "Indoor Humidity",4:"Outdoor temp" }

def recordIncomingMeasurements(writer):
    prev_value = 0L
    while True:
        if receiver.available():
            value = receiver.getReceivedValue()

            if value == prev_value:
                # we have already seen this measurement, so ignore it
                continue
            
            # decode byte3
            byte3 = (0xFF000000 & value) >> 24
            typeID = int((0xF0 & byte3) >> 4)
            seqNum = int((0x0F & byte3))

            # decode byte2 and byte1
            data = int((0x00FFFF00 & value) >> 8)

            # decode byte0
            checkSum = int((0x000000FF & value))

            calculatedCheckSum = 0xFF & (typeID + seqNum + data)

            if typeID == 4:
                # Handle float values that can be negative from -327.67 to +327.67
                # Bit 15 contains the sign flag,
                # the rest of the word (max 0x7FFF) contains the float value * 100
                floatResult = 0.0
                if (data & 0x8000 > 0):
                    # this should be a negative value
                    data &=~(1 << 15)
                    floatResult = -data
                else:
                    data &=~(1 << 15)
                    floatResult = data
                    
                data = floatResult/100.0
                    

            # Sanity checks on received data
            correctData = True
            if calculatedCheckSum != checkSum:
#                print("Incorrect checksum!")
                correctData = False
            elif typeID not in acceptedTypes:
#                print("Incorrect ID!")
                correctData = False
            elif seqNum > 15:
#                print("Incorrect SeqNum")
                correctData = False

            if correctData:
                timeValue = time.ctime()
                print(str.format("{0}: {1}={2} SeqNum={3}",timeValue,acceptedTypes[typeID],data,seqNum))
                prev_value = value
                writer.writerow({'Time':timeValue,acceptedTypes[typeID]:data})
                csvfile.flush()
                
            receiver.resetAvailable()
        

with open('results.csv', 'w') as csvfile:
    fieldNames = ['Time',acceptedTypes[1],acceptedTypes[2],acceptedTypes[3],acceptedTypes[4]]
    writer = csv.DictWriter(csvfile,fieldnames=fieldNames)
    writer.writeheader()

    recordIncomingMeasurements(writer)
