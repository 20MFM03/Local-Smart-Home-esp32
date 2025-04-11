#include <DHT.h>
#include <Arduino.h>

#define HEADER 0xAA
#define TAIL   0x55
#define CMD_SET_LED   0x01
#define CMD_SET_RGB   0x02
#define CMD_READ_DHT  0x03
#define CMD_STATUS    0x05
#define COMMON_ANODE true

const int LED_PINS[8] = {2, 3, 4, 5, 6, 7, 8, 12};

#define RED_PIN   9 
#define GREEN_PIN 10
#define BLUE_PIN  11
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
bool ledStates[8] = {false, false, false, false, false, false, false, false};
uint8_t currentR = 0, currentG = 0, currentB = 0;
unsigned long lastDHTReadTime = 0;
int16_t lastTemp = 0;
int16_t lastHum = 0;
unsigned long lastStatusTime = 0;
void sendFrame(byte cmd, byte len, byte *data) {
  byte chk = cmd ^ len;
  for (int i = 0; i < len; i++) {
    chk ^= data[i];
  }
  Serial.write(HEADER);
  Serial.write(cmd);
  Serial.write(len);
  for (int i = 0; i < len; i++) {
    Serial.write(data[i]);
  }
  Serial.write(chk);
  Serial.write(TAIL);
}
void sendStatusFrame() {
  byte payload[15];
  for (int i = 0; i < 8; i++) {
    payload[i] = ledStates[i] ? 1 : 0;
  }
  payload[8] = currentR;
  payload[9] = currentG;
  payload[10] = currentB;
  
  unsigned long now = millis();
  float t, h;
  if (now - lastDHTReadTime > 2000) {
    t = dht.readTemperature();
    h = dht.readHumidity();
    if (isnan(t) || isnan(h)) {
      Serial.println("DHT reading failed. Using default value 20 for temp and hum.");
      t = 20;
      h = 20;
      lastTemp = 20 * 100;
      lastHum = 20 * 100;
      lastDHTReadTime = now;
    } else {
      lastTemp = t * 100;
      lastHum = h * 100;
      lastDHTReadTime = now;
    }
  }
  payload[11] = highByte(lastTemp);
  payload[12] = lowByte(lastTemp);
  payload[13] = highByte(lastHum);
  payload[14] = lowByte(lastHum);
  
  sendFrame(CMD_STATUS, 15, payload);
}
void setLEDOutput(int index, bool state) {
  byte value = (COMMON_ANODE) ? (state ? LOW : HIGH) : (state ? HIGH : LOW);
  digitalWrite(LED_PINS[index], value);
  ledStates[index] = state;
  Serial.print("LED ");
  Serial.print(index);
  Serial.print(" set to ");
  Serial.println(state ? "ON" : "OFF");
}
void setRGBOutput(uint8_t r, uint8_t g, uint8_t b) {
  if (COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
  currentR = r; currentG = g; currentB = b;
  Serial.print("RGB set to: R=");
  Serial.print(r);
  Serial.print(" G=");
  Serial.print(g);
  Serial.print(" B=");
  Serial.println(b);
}
enum RxState { WAIT_HEADER, READ_CMD, READ_LEN, READ_DATA, READ_CHK, WAIT_TAIL };
RxState rxState = WAIT_HEADER;
byte rxCmd = 0;
byte rxLen = 0;
byte rxData[20];
byte rxIndex = 0;
byte rxChk = 0;

void processFrame(byte cmd, byte len, byte *data) {
  switch (cmd) {
    case CMD_SET_LED:
      if (len >= 2) {
        byte ledIndex = data[0];
        byte state = data[1];
        if (ledIndex < 8) {
          if (state == 2) { // toggle
            ledStates[ledIndex] = !ledStates[ledIndex];
            setLEDOutput(ledIndex, ledStates[ledIndex]);
          } else {
            setLEDOutput(ledIndex, state == 1);
          }
        }
        sendStatusFrame();
      }
      break;
    case CMD_SET_RGB:
      if (len >= 3) {
        setRGBOutput(data[0], data[1], data[2]);
        sendStatusFrame();
      }
      break;
    case CMD_READ_DHT:
      sendStatusFrame();
      break;
    default:
      Serial.print("Unknown command: ");
      Serial.println(cmd, HEX);
      break;
  }
}

void loopNano() {
  while (Serial.available()) {
    byte inByte = Serial.read();
    switch (rxState) {
      case WAIT_HEADER:
        if (inByte == HEADER) {
          rxState = READ_CMD;
          rxChk = 0;
        }
        break;
      case READ_CMD:
        rxCmd = inByte;
        rxChk ^= inByte;
        rxState = READ_LEN;
        break;
      case READ_LEN:
        rxLen = inByte;
        rxChk ^= inByte;
        rxIndex = 0;
        rxState = (rxLen == 0) ? READ_CHK : READ_DATA;
        break;
      case READ_DATA:
        rxData[rxIndex++] = inByte;
        rxChk ^= inByte;
        if (rxIndex >= rxLen)
          rxState = READ_CHK;
        break;
      case READ_CHK:
        if (inByte == rxChk)
          rxState = WAIT_TAIL;
        else {
          Serial.println("Checksum error");
          rxState = WAIT_HEADER;
        }
        break;
      case WAIT_TAIL:
        if (inByte == TAIL) {
          processFrame(rxCmd, rxLen, rxData);
        }
        rxState = WAIT_HEADER;
        break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino Nano (Slave) starting...");
  delay(2000);
  for (int i = 0; i < 8; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], COMMON_ANODE ? HIGH : LOW);
    ledStates[i] = false;
  }
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setRGBOutput(255, 255, 255);
  dht.begin();
}

void loop() {
  loopNano();
  if (millis() - lastStatusTime >= 500) {
    sendStatusFrame();
    lastStatusTime = millis();
  }
}
