#include <Adafruit_PWMServoDriver.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

WiFiUDP Udp;  // Creation of wifi Udp instance
char packetBuffer[255];
unsigned int localPort = 9999;

unsigned long lastSignalTime;  // Last signal from computer

const char* ssid = "VIA-MakerBot-01";
const char* password = "makerbotisfun";
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

void setPWM(int chan1, int chan2, bool state, uint16_t val) {
    Serial.println(val);
    if (state) {
        pwm.setPWM(chan1, 0, val);
        pwm.setPWM(chan2, 4096, 0);
    } else {
        pwm.setPWM(chan2, 0, val);
        pwm.setPWM(chan1, 4096, 0);
    }
}

// Value range: [-100, 100]
// 0 -> 100: Forward
// 0 -> -100: Backward
void controlWheels(int leftWheel, int rightWheel) {
    // TODO: Send pwm to wheel
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
        if (len > 0) packetBuffer[len - 1] = 0;
        char* token = strtok(packetBuffer, " ");
        if (strcasecmp(token,
                       "SET_CONTROL_IP")) {  // Packet to set controller's IP
            token = strtok(0, " ");
            controlIP.fromString(String(token));
        } else if (strcasecmp(token,
                              "CONTROL_WHEEL")) {  // Packet to set wheel speed
            token = strtok(0, " ");
            int leftWheel = atoi(token);
            int rightWheel = atoi(token);
            controlWheels(leftWheel, rightWheel);
        }
        lastSignalTime =
            millis();  // Save the last time receiving signal from controller
    }

    // Send some data: IMU, sensors?
    Udp.beginPacket(controlIP, 9999);
    char buf[20];
    int someData = 12345;
    sprintf(buf, "%d", someData);
    Udp.printf(buf);
    Udp.printf("\r\n");
    Udp.endPacket();
}
