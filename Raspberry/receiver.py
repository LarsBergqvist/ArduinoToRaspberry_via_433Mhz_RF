# Uses pi_switch from https://github.com/lexruee/pi-switch-python
# See pi_switch readme for details on setup

from pi_switch import RCSwitchReceiver
import time

receiver = RCSwitchReceiver()
receiver.enableReceive(2)

acceptedTypes = { 1 : "Light", 2 : "Temp [C]", 3: "Humidity [%]" }
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

        # Sanity checks on received data
        correctData = True
        if calculatedCheckSum != checkSum:
#            print("Incorrect checksum!")
            correctData = False
        elif data > 1023:
#            print("Incorrect data!")
            correctData = False
        elif typeID not in acceptedTypes:
#            print("Incorrect ID!")
            correctData = False
        elif seqNum > 15:
#            print("Incorrect SeqNum")
            correctData = False

        if correctData:
            print(str.format("{0}: {1}={2} SeqNum={3}",time.ctime(),acceptedTypes[typeID],data,seqNum))
            prev_value = value

        receiver.resetAvailable()
        
