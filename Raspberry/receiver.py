# Uses pi_switch from https://github.com/lexruee/pi-switch-python
# See pi_switch readme for details on setup

from pi_switch import RCSwitchReceiver

receiver = RCSwitchReceiver()
receiver.enableReceive(2)

while True:
    if receiver.available():
        value = receiver.getReceivedValue()

        # decode byte3
        byte3 = (0xFF000000 & value) >> 24
        arduinoID = int((0xF0 & byte3) >> 4)
        seqNum = int((0x0F & byte3))

        # decode byte2 and byte1
        data = int((0x00FFFF00 & value) >> 8)

        # decode byte0
        checkSum = int((0x000000FF & value))

        calculatedCheckSum = 0xFF & (arduinoID + seqNum + data)

        if calculatedCheckSum != checkSum:
            print("Incorrect checksum!")
            print(calculatedCheckSum,checkSum)
        elif data > 1023:
            print("Incorrect data!")
        elif arduinoID > 15:
            print("Incorrect ID!")
        elif seqNum > 15:
            print("Incorrect SeqNum")

        print(arduinoID,seqNum,data)

#        print("ArduinoID: ", arduinoID)
#        print("SeqNum: ", seqNum)
#        print("Data: ", data)
#        print("CheckSum: ", checkSum)


        receiver.resetAvailable()
        
