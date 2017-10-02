/*
 *
 * This doesn't really work. :-(
 * It ran for a bit on my digispark knockoff, and then failed spectacularly and smokily:
 *   The digispark was burned out. And Windows warned about a surge on the USB port. I had to
 *   reboot the computer to get the USB port working again.
 * The code might be useful for someone, though. I plan to port it to stm32 and see if it works.
 * I have a hunch that the problems had some connection to the fact that the Nunchuck is designed
 * for 3.3V and the Digispark runs at 5V.
 *
 */


//TODO: debounce direction switch

#include <DigiJoystick.h>
#include "ArduinoNunchuk.h"
#include <TinyWireM.h>

#define ROTATION_DETECTOR 1
#define DIRECTION_SWITCH_ANALOG  0 
#define DIRECTION_SWITCH_DIGITAL  0 
#define FORWARD           0

#define BEST_REASONABLE_RPM 120
#define MAX_USABLE_RPM 240
#define SLOWEST_REASONABLE_RPM 15

#define DIRECTION_THRESHOLD 875

uint8_t currentSpeed = 0x0;
uint8_t  currentRotationDetector = 0;
uint8_t directionSwitch;
uint32_t newDirectionTime = 0;
const uint32_t directionDebounceTime = 30;
uint32_t newRotationPulseTime = 0;
const uint32_t rotationDebounceTime = 10;
uint32_t lastPulse = 0;
const uint32_t shortestReasonableRotationTime = 1000 * 60 / BEST_REASONABLE_RPM;
const uint32_t shortestAllowedRotationTime = 1000 * 60 / MAX_USABLE_RPM;
const uint32_t longestReasonableRotationTime = 1000 * 60 / SLOWEST_REASONABLE_RPM;

ArduinoNunchuk nunchuk = ArduinoNunchuk();

void setup() {
  DigiJoystick.delay(500);
  nunchuk.init();
  pinMode(ROTATION_DETECTOR, INPUT);//TODO: no pullup on final version
//  pinMode(DIRECTION_SWITCH_DIGITAL, INPUT);//PULLUP OFF? need resistor divider to avoid reset
  directionSwitch = analogRead(DIRECTION_SWITCH_ANALOG) > DIRECTION_THRESHOLD;
  currentRotationDetector = digitalRead(ROTATION_DETECTOR);   
}


void loop() {
  nunchuk.update();
  
  DigiJoystick.setX((byte) nunchuk.analogX); // scroll X left to right repeatedly
  DigiJoystick.setY((byte)(255-nunchuk.analogY));
//  DigiJoystick.setX((byte) (analogRead(0)/4)); // scroll X left to right repeatedly
//  DigiJoystick.setXROT((byte) map(nunchuk.accelX,255,700,0,255));
//  DigiJoystick.setYROT((byte) map(nunchuk.accelY,255,850,0,255));
//  DigiJoystick.setZROT((byte) map(nunchuk.accelZ,255,750,0,255));
  int buttonByte = 0;
  bitWrite(buttonByte, 0, nunchuk.zButton);
  bitWrite(buttonByte, 1, nunchuk.cButton);
  directionSwitch = analogRead(DIRECTION_SWITCH_ANALOG) > DIRECTION_THRESHOLD;

  uint32_t dt = millis() - lastPulse;

  if (dt > longestReasonableRotationTime)
    currentSpeed = 0;

  if (digitalRead(ROTATION_DETECTOR) != currentRotationDetector) {
    if(millis() >= newRotationPulseTime+rotationDebounceTime) {
      currentRotationDetector = ! currentRotationDetector;
      newRotationPulseTime = millis();
      if (currentRotationDetector) {
        lastPulse = millis();
        if (dt >= shortestAllowedRotationTime) {
          if (dt > longestReasonableRotationTime) {
              currentSpeed = 0;
          }
          else {
            if (dt < shortestReasonableRotationTime) {
              dt = shortestReasonableRotationTime;
            }
            currentSpeed = 0x7f * shortestReasonableRotationTime / dt;
          } 
        }
      }
    }
  }

  DigiJoystick.setSLIDER(directionSwitch ?(byte) (128+currentSpeed) : (byte)(128-currentSpeed));

  //bitWrite(buttonByte, 2, directionSwitch); 
  DigiJoystick.setButtons((byte) buttonByte, (byte) 0); 
  
  DigiJoystick.delay(10); 
}
