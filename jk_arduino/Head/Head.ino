#include "Motor.h"

int MotorLP1 = 2;
int MotorLP2 = 4;
int MotorLP3 = 3;
int MotorRP1 = 5;
int MotorRP2 = 7;
int MotorRP3 = 6;

Motor motorPan;
Motor motorTilt;

long cmdExpires = 0;
int baseLeftCmd = 0;
int baseRightCmd = 0;
int headPanCmd = 0;
int headTiltCmd = 0;

void setup() {
  Serial.begin(115200);
  Serial.println("Head controller");

  motorTilt = Motor(1, MotorLP1, MotorLP2, MotorLP3);
  motorPan = Motor(2, MotorRP1, MotorRP2, MotorRP3);
  
  motorPan.setUp();
  motorTilt.setUp();
}

int bufIdx = 0;
char buf[64];

void parseCmd () {
  bufIdx = 0;

  char _cmd[64];
  int _cmdIdx = 0;
  
  int cmdIdx = 0;

  while (buf[bufIdx] != ';' && bufIdx < 64) {
    if (buf[bufIdx] == ',') {
      _cmd[_cmdIdx++] = '\0';
      if (cmdIdx == 0) {
        baseLeftCmd = atoi(_cmd);
      } else if (cmdIdx == 1) {
        baseRightCmd = atoi(_cmd);
      } else if (cmdIdx == 2) {
        headPanCmd = atoi(_cmd);
      } else if (cmdIdx == 3) {
        headTiltCmd = atoi(_cmd);
      }

      _cmdIdx = 0;
      cmdIdx++;
      bufIdx++;
    } else {
      _cmd[_cmdIdx++] = buf[bufIdx++];
    }
  }
  
  bufIdx = 0;
  cmdExpires = millis() + 250;
}

void loop() {
  //100,100,0,0,;
  //wheel left, wheel right, neck pan, neck tilt, ;
  uint8_t i = 0;
  while (Serial.available() > 0 && i < 32) {
    i++;
    char c = Serial.read();
    buf[bufIdx++] = c;
    if (c == ';') {
      parseCmd();
    }
  }

  if (millis() < cmdExpires) {
    motorPan.step(headPanCmd);
    motorTilt.step(headTiltCmd);
  } else {
    motorPan.step(0);
    motorTilt.step(0);
  }

  delay(50);
}
