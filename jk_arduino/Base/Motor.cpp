#include <Arduino.h>
#include "Motor.h"

Motor::Motor() {
  
}

Motor::Motor(int id, int pinEnable, int pinDrive1, int pinDrive2) {
  // speedSet set to a percentage of max voltage to pins
  this->speedSet = 0;
  this->encoderTickCount = 0;
  this->id = id;
  this->pinEnable = pinEnable;
  this->pinDrive1 = pinDrive1;
  this->pinDrive2 = pinDrive2;
}

Motor::~Motor() {
  this->stop();
}

void Motor::changeMotorDirection() {
  int oldPinDrive1 = this->pinDrive1;
  int oldPinDrive2 = this->pinDrive2;
  this->pinDrive1 = oldPinDrive2;
  this->pinDrive2 = oldPinDrive1;
}

void Motor::setPinSpeed() {
  int s = abs(speedCurrent);
  if (s > maxSpeed) {
    s = maxSpeed;
  }
  int pinStrength = int(255.0 * (s / 100.0));
  analogWrite(pinEnable, pinStrength);
}

void Motor::setUp() {
  pinMode(pinEnable, OUTPUT);
  pinMode(pinDrive1, OUTPUT);
  pinMode(pinDrive2, OUTPUT);

  setPinSpeed();
}

void Motor::forward(int speed = 100) {
  // given speed should be a percentage of total speed so that we can tell it to start slowly
  if (speed < minSpeed) {
    analogWrite(pinEnable, LOW);
    digitalWrite(pinDrive1, LOW);
    digitalWrite(pinDrive2, LOW);
  } else {
    digitalWrite(pinDrive1, HIGH);
    digitalWrite(pinDrive2, LOW);
  }

  isMovingForward = true;
  isMovingBackward = false;
}

void Motor::backward(int speed = 100) {
  if (speed < minSpeed) {
    analogWrite(pinEnable, LOW);
    digitalWrite(pinDrive1, LOW);
    digitalWrite(pinDrive2, LOW);
  } else {
    digitalWrite(pinDrive1, LOW);
    digitalWrite(pinDrive2, HIGH);
  }

  isMovingForward = false;
  isMovingBackward = true;
}

void Motor::step(int speed = 100) {
  speedSet = speed;
  
  if (speedSet != speedCurrent) {
    int dir = 1;
    if (speedSet < speedCurrent) {
      dir = -1;
    }

    if (abs(speedSet - speedCurrent) < speedInc) {
      speedCurrent = speedSet;
    } else {
      speedCurrent = speedCurrent + (dir * speedInc);
    }
    
    setPinSpeed();
  }

  /*
  analogWrite(pinEnable, LOW);
  digitalWrite(pinDrive1, LOW);
  digitalWrite(pinDrive2, LOW);
  */
  
  if (speedCurrent > 0) {
    forward(abs(speedCurrent));
  } else if (speedCurrent < 0) {
    backward(abs(speedCurrent));
  } else {
    stop();
  }
}

void Motor::stop() {
  this->speedSet = 0;
  analogWrite(pinEnable, LOW);
  digitalWrite(pinDrive1, LOW);
  digitalWrite(pinDrive2, LOW);

  isMovingForward = false;
  isMovingBackward = false;
}

int Motor::getEncoderTickCount() {
  return encoderTickCount;
}

int Motor::getPreviousEncoderTickCount() {
  float result = previousEncoderTickCount;
  previousEncoderTickCount = encoderTickCount;
  return result;
}

int Motor::getEncoderTickCountDelta() {
  float tickCount = getEncoderTickCount();
  float tickCountPrevious = getPreviousEncoderTickCount();
  return tickCount - tickCountPrevious;
}

void Motor::resetEncoderTickCount() {
  encoderTickCount = 0;
}

void Motor::incrementEncoderTickCount() {
  encoderTickCount++;
}

void Motor::decrementEncoderTickCount() {
  encoderTickCount--;
}

bool Motor::isFlagged() {
  return flagExecute;
}

void Motor::clearPreparedCommand() {
  flagExecute = false;
  flagExecuteSpeed = 0;
  flagExecuteDuration = 0;
  flagExecuteExpiration = millis() - 2000;
}

void Motor::prepareCommand(int speedSet, int duration) {
  flagExecute = true;
  flagExecuteSpeed = speedSet;
  flagExecuteDuration = duration;
  flagExecuteExpiration = millis() + duration;
  
  lastPreparedCommand[0] = speedSet;
  lastPreparedCommand[1] = duration;
}

void Motor::executePreparedCommand() {
  if (flagExecute == true && millis() < flagExecuteExpiration) {
    this->step(flagExecuteSpeed);
  } else if (millis() > flagExecuteExpiration) {
    this->step(0);
    flagExecute = false;
  } else {
    this->step(0);
  }
}

void Motor::getLastMotorCommand(int *cmd) {
  cmd[0] = lastPreparedCommand[0];
  cmd[1] = lastPreparedCommand[1];
}
