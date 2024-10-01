String command = "";
String value = "";
int delimiterIndex;

#include <Servo.h>
Servo Sservo;
Servo Fservo;
Servo Bservo;

#include <AccelStepper.h>
const int dirPin = 6;
const int stepPin = 5;
const int enablePin = 7;

AccelStepper stepper(1, stepPin, dirPin);

int switchPin = 11;

int potPin = A0;

int ledYPin = 2;
int ledGPin = 3;
int ledRPin = 4;

bool sampleEnabled = false;
bool motorEnabled = false;
unsigned long speed = 1000;
static int lastSwitchState = 0;
int lastPotValue = -1;
float xinc = 0.0;
float fraction_sample = 0.0;
float xwindow = 0.0;
const float stepAngle = 1.8;
const float pulleyRadius = 0.0122;       
const float largerPulleyRadius = 0.03667;  
const float beltRadius = 0.054;        
const int stepsPerRevolution = 200;
float beltMovement = 0.0;
float xofs = 0.0;
int steps = 0;
float supplyLength = 1;
float CF = 1.253;


float beltLength = 2.07; 


enum SlowLoopState {
  INIT_1,
  INITIAL_WAIT_TIME,
  MOVE_STOP_STEPPER_XOFS_2,
  MOVE_SAMPLE_FLAP_3,
  WAIT_BEFORE_SAMPLING_4,
  START_STEPPER_MOTOR_5,
  MOVE_STOP_STEPPER_XINC_6,
  MOVE_BULK_FLAP_7,
  WAIT_BEFORE_SAMPLING_8,
  START_STEPPER_MOTOR_9,
  MOVE_STEPPER_REST_10,
};

SlowLoopState slowLoopState = INIT_1;
bool stepperStartedMoving = false;
bool stepperStoppedMoving = false;
bool stepperMovementStarted = false;
bool stepperMovementCompleted = false;
bool initialSettings = false;
bool frontServoPosition = false;
bool bulServoPosition = false;

bool readInitialPosition1 = false;
bool readInitialPosition2 = false;
bool readInitialPosition5 = false;
bool readInitialPosition6 = false;
bool readInitialPosition7 = false;
bool readInitialPosition8 = false;
bool readInitialPosition9 = false;
bool readInitialPosition10 = false;
bool readInitialPosition21 = false;

bool moveFlap3 = false;
bool startMotor5 = false;
bool moveFlap6 = false;
bool startMotor7 = false;
bool moveFlap7 = false;
bool startMotor9 = false;
bool firstLoop = true;
bool readTime = false;

int initialPosition1;
int initialPosition2;
int previousPosition2;
int initialPosition5;
int initialPosition6;
int previousPosition5;
int previousPosition6;
int initialPosition7;
int initialPosition8;
int initialPosition9;
int initialPosition10;
int initialPosition21;

unsigned long startTime;
unsigned long waitTime;
float angularSpeedSteps;
float angularSpeedRadians;
float linearSpeed;
float angularSpeedLargerPulley;
float linearBeltSpeed;
int currentPosition;
int tolerance = 1;
unsigned long lastCheckTime = 0;
unsigned long delayInterval = 50;



void setup() {
  Serial.begin(57600);
  randomSeed(analogRead(0));

  Sservo.attach(8);
  Fservo.attach(9);
  Bservo.attach(10);

  pinMode(enablePin, OUTPUT);
  stepper.enableOutputs();

  stepper.setMaxSpeed(1000);
  stepper.setAcceleration(100);
  stepper.setSpeed(speed);

  pinMode(switchPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  pinMode(ledYPin, OUTPUT);
  pinMode(ledGPin, OUTPUT);
  pinMode(ledRPin, OUTPUT);

  Serial.println("LEDY:ON");
}


void loop() {
  fastLoop();

  if (sampleEnabled) {
    slowLoop();
  }
}



void fastLoop() {
  bool currentSwitchState = digitalRead(switchPin);
  if (currentSwitchState != lastSwitchState) {
    lastSwitchState = currentSwitchState;
    if (currentSwitchState == LOW) {
      Serial.println("SAMPLE:ON");
    } else {
      Serial.println("SAMPLE:OFF");
    }
  }

  if (Serial.available() > 0) {
    String received = Serial.readStringUntil('\n');
    delimiterIndex = received.indexOf(':');
    if (delimiterIndex != -1) {
      command = received.substring(0, delimiterIndex);
      value = received.substring(delimiterIndex + 1);

      if (command.equals("xinc")) {
        float f_value = value.toFloat();
        xinc = f_value;
      } else if (command.equals("fraction_sample")) {
        float f_value = value.toFloat();
        fraction_sample = f_value;
      }

      else if (command.equals("SSERVO")) {
        int servoS_position = value.toInt();
        if (servoS_position >= 0 && servoS_position <= 180) {
          Sservo.write(servoS_position);
        }
      }

      else if (command.equals("FSERVO")) {
        int servoF_position = value.toInt();
        if (servoF_position >= 0 && servoF_position <= 180) {
          Fservo.write(servoF_position);
        }
      }

      else if (command.equals("BSERVO")) {
        int servoB_position = value.toInt();
        if (servoB_position >= 0 && servoB_position <= 180) {
          Bservo.write(servoB_position);
        }
      }

      else if (command.equals("STEPPERSPEED")) {
        speed = value.toInt();
      }

      else if (command.equals("STEPPER")) {
        if (value.equals("1")) {
          digitalWrite(enablePin, LOW);
          motorEnabled = true;
        } else if (value.equals("0")) {
          digitalWrite(enablePin, HIGH);
          motorEnabled = false;
        }
      }

      else if (command.equals("LEDY")) {
        digitalWrite(ledYPin, value.toInt());
      }

      else if (command.equals("LEDG")) {
        digitalWrite(ledGPin, value.toInt());
      }

      else if (command.equals("LEDR")) {
        digitalWrite(ledRPin, value.toInt());
      }

      else if (command.equals("SAMPLE")) {
        if (value.equals("1")) {
          Serial.println("LEDG:ON");
          sampleEnabled = true;
        } else if (value.equals("0")) {
          Serial.println("STEPPER:OFF");
          Serial.println("LEDG:OFF");
          Serial.println("LEDR:OFF");
          sampleEnabled = false;
          slowLoopState = INIT_1;
          SlowLoopState slowLoopState = INIT_1;

          stepperStartedMoving = false;
          frontServoPosition = false;
          bulServoPosition = false;
          readInitialPosition1 = false;
          initialSettings = false;

          firstLoop = true;

          stepperMovementStarted = false;
          stepperMovementCompleted = false;
          readInitialPosition2 = false;
          stepperStoppedMoving = false;

          moveFlap3 = false;

          stepperMovementStarted = false;

          stepperStartedMoving = false;
          startMotor5 = false;
          readInitialPosition5 = false;

          stepperMovementStarted = false;
          stepperMovementCompleted = false;
          readInitialPosition6 = false;
          stepperStoppedMoving = false;

          moveFlap7 = false;

          stepperMovementStarted = false;

          stepperStartedMoving = false;
          startMotor9 = false;
          readInitialPosition9 = false;

          stepperMovementStarted = false;
          stepperMovementCompleted = false;
          readInitialPosition10 = false;
        }
      }
    }

    if (xinc != 0.0 && fraction_sample != 0.0) {
      xwindow = xinc / fraction_sample;
      float maxOffset = xwindow - xinc;
      int maxOffsetInt = maxOffset * 1000;
      int randomOffsetInt = random(maxOffsetInt + 1);
      xofs = randomOffsetInt / 1000.0;
    }
  }

  int currentPotValue = analogRead(potPin);
  int mappedSpeed = map(currentPotValue, 0, 1023, 0, 1000);
  if (abs(lastPotValue - currentPotValue) > 50) {
    Serial.print("STEPPERSPEED:");
    Serial.println(mappedSpeed);
    lastPotValue = currentPotValue;
  }

  if (motorEnabled) {
    stepper.setSpeed(speed);
    stepper.runSpeed();
  }
}



void slowLoop() {
  
  switch (slowLoopState) {
    case INIT_1:
      if (!motorEnabled) {
        if (!initialSettings) {
          Serial.println("STEPPER:ON");
          Serial.println("SERVOS:110");
          Serial.println("SERVOF:100");
          initialSettings = true;
        }
      }

      if (!readInitialPosition1) {
        initialPosition1 = stepper.currentPosition();
        steps = calculateStepsForBeltMovement(xwindow);
        Serial.print("WINDOW IN STEPS:");
        Serial.println(steps);
        readInitialPosition1 = true;
      }
      stepperStartedMoving = waitForStepperMovement(stepper, initialPosition1);
      frontServoPosition = isServoAtPosition(Fservo, 100, tolerance);
      bulServoPosition = isServoAtPosition(Sservo, 110, tolerance);
      if (stepperStartedMoving && frontServoPosition && bulServoPosition) {
        stepperStartedMoving = false;
        frontServoPosition = false;
        bulServoPosition = false;
        readInitialPosition1 = false;
        initialSettings = false;
        if (firstLoop) {
          slowLoopState = INITIAL_WAIT_TIME;
          firstLoop = false;
        } else {
        slowLoopState = MOVE_STOP_STEPPER_XOFS_2;
        }
        Serial.println("KONEC 1");
      }
      break;

    
    case INITIAL_WAIT_TIME:
      if (!stepperMovementCompleted) {
        if (!stepperMovementStarted) {
          steps = calculateStepsForBeltMovement(supplyLength);
          if (!readInitialPosition21) {
            initialPosition21 = stepper.currentPosition();
            readInitialPosition21 = true;
          }
          stepperMovementStarted = true;
        } else {
          if (abs(stepper.currentPosition() - initialPosition21) >= abs(steps)) {
            Serial.println("LEDR:ON");
            stepperMovementCompleted = true;
          }
        }
      } else {
        stepperMovementStarted = false;
        stepperMovementCompleted = false;
        readInitialPosition21 = false;
        Serial.println("POCATECNI CEKANI");
        slowLoopState = MOVE_STOP_STEPPER_XOFS_2;
      }
      break;


    case MOVE_STOP_STEPPER_XOFS_2:
      if (!stepperMovementCompleted) {
        if (!stepperMovementStarted) {
          steps = calculateStepsForBeltMovement(xofs);
          Serial.print("RANDOM OFFSET:");
          Serial.println(xofs);
          Serial.print("XOFS IN STEPS:");
          Serial.println(steps);
          if (!readInitialPosition2) {
            initialPosition2 = stepper.currentPosition();
            readInitialPosition2 = true;
          }
          stepperMovementStarted = true;
        } else {
          if (abs(stepper.currentPosition() - initialPosition2) >= abs(steps)) {
            Serial.println("STEPPER:OFF");
            previousPosition2 = stepper.currentPosition();
            lastCheckTime = millis();
            stepperMovementCompleted = true;
          }
        }
      } else {
        stepperStoppedMoving = hasStepperStopped(stepper, previousPosition2, lastCheckTime, delayInterval);
        if (stepperStoppedMoving) {
          stepperMovementStarted = false;
          stepperMovementCompleted = false;
          readInitialPosition2 = false;
          stepperStoppedMoving = false;
          slowLoopState = MOVE_SAMPLE_FLAP_3;
          Serial.println("KONEC 2");
          }
      }
      break;


    case MOVE_SAMPLE_FLAP_3:
      if (!moveFlap3) {
        Serial.println("SERVOS:60");
        moveFlap3 = true;
        }
      if (isServoAtPosition(Sservo, 60, tolerance)) {
        moveFlap3 = false;
        slowLoopState = WAIT_BEFORE_SAMPLING_4;
        Serial.println("KONEC 3");
      }

    
    case WAIT_BEFORE_SAMPLING_4:
      if (!stepperMovementStarted) {
        startTime = millis();
        stepperMovementStarted = true;
      }
      waitTime = millis() - startTime;
      if (waitTime >= 1500) {
        stepperMovementStarted = false;
        slowLoopState = START_STEPPER_MOTOR_5;
        Serial.println("KONEC 4");
      }
      break;


    case START_STEPPER_MOTOR_5:
      if (!startMotor5) {
        Serial.println("STEPPER:ON");
        startMotor5 = true;
      }
      if (!readInitialPosition5) {
        initialPosition5 = stepper.currentPosition();
        readInitialPosition5 = true;
      }
      stepperStartedMoving = waitForStepperMovement(stepper, initialPosition5);
      if (stepperStartedMoving) {
        stepperStartedMoving = false;
        startMotor5 = false;
        readInitialPosition5 = false;
        slowLoopState = MOVE_STOP_STEPPER_XINC_6;
        Serial.println("KONEC 5");
      }


    case MOVE_STOP_STEPPER_XINC_6:
      if (!stepperMovementCompleted) {
        if (!stepperMovementStarted) {
          steps = calculateStepsForBeltMovement(xinc);
          Serial.print("XINC IN STEPS:");
          Serial.println(steps);
          if (!readInitialPosition6) {
            initialPosition6 = stepper.currentPosition();
            readInitialPosition6 = true;
          }
          stepperMovementStarted = true;
        } else {
          if (abs(stepper.currentPosition() - initialPosition6) >= abs(int(steps))) {
            Serial.println("STEPPER:OFF");
            previousPosition6 = stepper.currentPosition();
            lastCheckTime = millis();
            stepperMovementCompleted = true;
          }
        }
      } else {
        stepperStoppedMoving = hasStepperStopped(stepper, previousPosition6, lastCheckTime, delayInterval);
        if (stepperStoppedMoving) {
          stepperMovementStarted = false;
          stepperMovementCompleted = false;
          readInitialPosition6 = false;
          stepperStoppedMoving = false;
          slowLoopState = MOVE_BULK_FLAP_7;
          Serial.println("KONEC 6");
          }
      }
      break;


    case MOVE_BULK_FLAP_7:
      if (!moveFlap7) {
        Serial.println("SERVOS:110");
        moveFlap7 = true;
        }
      if (isServoAtPosition(Sservo, 110, tolerance)) {
        moveFlap7 = false;
        slowLoopState = WAIT_BEFORE_SAMPLING_8;
        Serial.println("KONEC 7");
      }


    case WAIT_BEFORE_SAMPLING_8:
      if (!stepperMovementStarted) {
        startTime = millis();
        stepperMovementStarted = true;
      }
      waitTime = millis() - startTime;
      if (waitTime >= 1500) {
        stepperMovementStarted = false;
        slowLoopState = START_STEPPER_MOTOR_9;
        Serial.println("KONEC 8");
      }
      break;


    case START_STEPPER_MOTOR_9:
      if (!startMotor9) {
        Serial.println("STEPPER:ON");
        startMotor9 = true;
      }
      if (!readInitialPosition9) {
        initialPosition9 = stepper.currentPosition();
        readInitialPosition9 = true;
      }
      stepperStartedMoving = waitForStepperMovement(stepper, initialPosition9);
      if (stepperStartedMoving) {
        stepperStartedMoving = false;
        startMotor9 = false;
        readInitialPosition9 = false;
        slowLoopState = MOVE_STEPPER_REST_10;
        Serial.println("KONEC 9");
      }

    case MOVE_STEPPER_REST_10:
      if (!stepperMovementCompleted) {
        if (!stepperMovementStarted) {
          steps = calculateStepsForBeltMovement((xwindow - xinc - xofs));
          Serial.print("REST IN STEPS:");
          Serial.println(steps);
          if (!readInitialPosition10) {
            initialPosition10 = stepper.currentPosition();
            readInitialPosition10 = true;
          }
          stepperMovementStarted = true;
        } else {
          if (abs(stepper.currentPosition() - initialPosition10) >= abs(steps)) {
            stepperMovementCompleted = true;
          }
        }
      } else {
        stepperMovementStarted = false;
        stepperMovementCompleted = false;
        readInitialPosition10 = false;
        Serial.println("DOKONCEN LOOP");
        Serial.println("KONEC 10");
        slowLoopState = INIT_1;
      }
      break;
  }
}



bool isServoAtPosition(Servo& servo, int targetPosition, int tolerance) {
  int currentServoPosition = servo.read();
  if (currentServoPosition >= (targetPosition - tolerance) && currentServoPosition <= (targetPosition + tolerance)) {
    return true;
  }
  return false;
}

bool waitForStepperMovement(AccelStepper& stepper, int initialPosition) {
  if (stepper.currentPosition() != initialPosition) {
      return true;
  }
  return false;
}

bool hasStepperStopped(AccelStepper& stepper, int& previousPosition, unsigned long& lastCheckTime, unsigned long delayInterval) {
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= delayInterval) {
        int currentPosition = stepper.currentPosition();
        lastCheckTime = currentTime;
        if (currentPosition == previousPosition) {
            return true;
        } else {
            previousPosition = currentPosition;
            return false;
        }
    }
    return false;
}

int calculateStepsForBeltMovement(float beltMovement) {
  angularSpeedSteps = (float)stepper.speed();
  angularSpeedRadians = angularSpeedSteps * stepAngle * DEG_TO_RAD;
  linearSpeed = angularSpeedRadians * pulleyRadius;
  angularSpeedLargerPulley = linearSpeed / largerPulleyRadius;
  linearBeltSpeed = angularSpeedLargerPulley * beltRadius;
  steps = (int)((beltMovement / linearBeltSpeed) * CF * stepper.speed());
  return steps;
}