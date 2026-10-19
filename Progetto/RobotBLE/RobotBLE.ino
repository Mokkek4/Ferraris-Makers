#include <ArduinoBLE.h>

// TB6612FNG - Motore A
const int pinPWMA = 9;
const int pinAIN1 = 4;
const int pinAIN2 = 5;

// TB6612FNG - Motore B
const int pinPWMB = 10;
const int pinBIN1 = 7;
const int pinBIN2 = 8;

const int pinSTBY = 6;

// Bluetooth BLE
BLEService robotService("19B10000-E8F2-537E-4F6C-D104768A1214");

BLEStringCharacteristic commandCharacteristic(
  "19B10001-E8F2-537E-4F6C-D104768A1214",
  BLEWrite,
  20
);

int velocita = 100;

void stopMotori() {
  analogWrite(pinPWMA, 0);
  analogWrite(pinPWMB, 0);
}

void motori(int dirA, int dirB) {
  // dir: 1 avanti, -1 indietro, 0 fermo
  digitalWrite(pinAIN1, dirA > 0 ? HIGH : LOW);
  digitalWrite(pinAIN2, dirA < 0 ? HIGH : LOW);

  digitalWrite(pinBIN1, dirB > 0 ? HIGH : LOW);
  digitalWrite(pinBIN2, dirB < 0 ? HIGH : LOW);

  analogWrite(pinPWMA, dirA == 0 ? 0 : velocita);
  analogWrite(pinPWMB, dirB == 0 ? 0 : velocita);
}

void eseguiComando(String cmd) {
  cmd.trim();

  if (cmd == "F") {
    motori(1, 1);
  } else if (cmd == "B") {
    motori(-1, -1);
  } else if (cmd == "L") {
    motori(-1, 1);
  } else if (cmd == "R") {
    motori(1, -1);
  } else if (cmd == "S") {
    stopMotori();
  } else if (cmd.startsWith("V")) {
    velocita = constrain(cmd.substring(1).toInt(), 0, 255);
  }
}

void setup() {
  pinMode(pinPWMA, OUTPUT);
  pinMode(pinAIN1, OUTPUT);
  pinMode(pinAIN2, OUTPUT);

  pinMode(pinPWMB, OUTPUT);
  pinMode(pinBIN1, OUTPUT);
  pinMode(pinBIN2, OUTPUT);

  pinMode(pinSTBY, OUTPUT);

  digitalWrite(pinSTBY, HIGH);
  stopMotori();

  Serial.begin(115200);

  if (!BLE.begin()) {
    while (true) {}
  }

  BLE.setLocalName("RobotBLE");
  BLE.setAdvertisedService(robotService);

  robotService.addCharacteristic(commandCharacteristic);
  BLE.addService(robotService);

  commandCharacteristic.writeValue("S");
  BLE.advertise();

  Serial.println("RobotBLE pronto");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.println("Telefono connesso");

    while (central.connected()) {
      BLE.poll();

      if (commandCharacteristic.written()) {
        String cmd = commandCharacteristic.value();
        Serial.println(cmd);
        eseguiComando(cmd);
      }
    }

    // Sicurezza: ferma il robot quando si disconnette
    stopMotori();
    Serial.println("Telefono disconnesso");
  }
}
