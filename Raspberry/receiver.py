# Uses pi_switch from https://github.com/lexruee/pi-switch-python
# See pi_switch readme for details on setup

from pi_switch import RCSwitchReceiver

receiver = RCSwitchReceiver()
receiver.enableReceive(2)

acceptedIds = [3,4]
prev_value = 0L
while True:
    if receiver.available():
        value = receiver.getReceivedValue()

        if value == prev_value:
            # we have already seen this measurement, so ignore it
            continue
        
        # decode byte3
        byte3 = (0xFF000000 & value) >> 24
        arduinoID = int((0xF0 & byte3) >> 4)
        seqNum = int((0x0F & byte3))

        # decode byte2 and byte1
        data = int((0x00FFFF00 & value) >> 8)

        # decode byte0
        checkSum = int((0x000000FF & value))

        calculatedCheckSum = 0xFF & (arduinoID + seqNum + data)

        # Sanity checks on received data
        if calculatedCheckSum != checkSum:
            print("Incorrect checksum!")
        elif data > 1023:
            print("Incorrect data!")
        elif arduinoID not in acceptedIds:
            print("Incorrect ID!")
        elif seqNum > 15:
            print("Incorrect SeqNum")
        else:
            print("OK data!", "ArduinoID:",arduinoID,"SeqNum:",seqNum,"Measurement:",data)
            prev_value = value

        receiver.resetAvailable()
        
