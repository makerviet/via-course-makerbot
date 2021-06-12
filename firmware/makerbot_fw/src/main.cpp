#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define DC_Motor_LEFT 8, 9
#define DC_Motor_RIGHT 10, 11
// #define DC_Motor_3
// #define DC_Motor_4
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

WiFiUDP Udp;  // Creation of wifi Udp instance
char packetBuffer[255];
unsigned int localPort = 9999;

unsigned long lastSignalTime;  // Last signal from computer

const char *ssid = "VIA-MakerBot-01";
const char *password = "makerbotisfun";
IPAddress netMsk(255, 255, 255, 0);
IPAddress apIP(192, 168, 4, 200);
IPAddress ipMakerbot(192, 168, 4, 100);
IPAddress controlIP(192, 168, 4, 200);

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    WiFi.mode(WIFI_STA);  // ESP-32 as client
    WiFi.config(ipMakerbot, apIP, netMsk);
    Udp.begin(localPort);

    // Motors
    pwm.begin();
    pwm.setOscillatorFrequency(27000000);
    pwm.setPWMFreq(1000);
    Wire.setClock(400000);
}

// 'state' is the motor directions, the real direction also depends on how
// mottor wires are connected to the motorshield
void setPWM(int chan1, int chan2, bool state, uint16_t val) {
    // Serial.print(val);
    // Serial.print("\t");
    // Serial.println(state);
    if (state)  // state = 1 clockwise rotation
    {
        pwm.setPWM(chan1, 0, val);
        pwm.setPWM(chan2, 4096, 0);
    } else  // state = 0 couter-clockwise rotation
    {
        pwm.setPWM(chan2, 0, val);
        pwm.setPWM(chan1, 4096, 0);
    }
}

// Value range: [-100, 100]
// 0 -> 100: Forward
// 0 -> -100: Backward
void controlWheels(int16_t leftWheel, int16_t rightWheel) {

    leftWheel = leftWheel * 40.96;
    rightWheel = rightWheel * 40.96;    

    //"0x8000 & leftWheel" is to get the signed bit of the speed varible
    // only works with 16 bits integer(int16_t), if you want to change
    // variable type to int (32 bits), change the value 0x8000 to 0x800000
    setPWM(DC_Motor_LEFT, 0x8000 & leftWheel, abs(leftWheel));
    setPWM(DC_Motor_RIGHT, 0x8000 & rightWheel, abs(rightWheel));
}

void loop() {
    // Receive control commands
    int packetSize = Udp.parsePacket();  // Size of packet to receive
    if (packetSize) {                    // If we received a package

        // Stop if the control signal is lost
        if (millis() - lastSignalTime > 1000) {
            controlWheels(0, 0);
        }

        // Parse UDP packet
        int len = Udp.read(packetBuffer, 255);
        if (len > 0) packetBuffer[len] = 0;
        char *token = strtok(packetBuffer, " ");
        if (strcasecmp(token, "SET_CONTROL_IP") == 0) {  // Packet to set controller's IP
            token = strtok(NULL, " ");
            controlIP.fromString(String(token));
        } else if (strcasecmp(token, "CONTROL_WHEEL") == 0) {  // Packet to set wheel speed
            token = strtok(NULL, " ");
            int16_t leftWheel = atoi(token);
            token = strtok(NULL, " ");
            int16_t rightWheel = atoi(packetBuffer);
            controlWheels(leftWheel, rightWheel);
        }
        lastSignalTime =
            millis();  // Save the last time receiving signal from controller
    }

    // Send some data: IMU, sensors?
    // Udp.beginPacket(controlIP, 9999);
    // char buf[20];
    // int someData = 12345;
    // sprintf(buf, "%d", someData);
    // Udp.printf(buf);
    // Udp.printf("\r\n");
    // Udp.endPacket();
}
