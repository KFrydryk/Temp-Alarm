#include <arduino.h>
#include <OneWire.h>
#include "PinChangeInt.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"

#define BUTTON_DEBOUNCE_TIME 100
#define ENCODER_DEBOUNCE_TIME 3

const byte addr[] = {0x28, 0x31, 0x56, 0xF5, 0x13, 0x19, 0x1, 0x93};
LiquidCrystal_I2C lcd(0x3F, 16, 2);

OneWire ds(10); // on pin 10 (a 4.7K resistor is necessary)

float temp;
bool ConversionStarted;
volatile int buttonCounter = 0;
volatile int encoderVal = 0;
unsigned long time = 0;
int setTemp = 0;
long setTimer = 0;
bool tempAlarmFlag = true;
bool tempLoFLAG = false;
bool tempAlarm = false;
bool timeAlarmFlag = true;
bool timeAlarm = false;
unsigned long startTime = 0;

int ReadTemp()
{
  byte data[12];
  if (!ConversionStarted)
  {
    ds.reset();
    ds.select(addr);
    ds.write(0x44, 0); // start conversion, with parasite power off at the end
    ConversionStarted = true;
    return 1;
  }
  if (ds.read() != 0)
  {
    ConversionStarted = false;
    ds.reset();
    ds.select(addr);
    ds.write(0xBE); // Read Scratchpad

    for (int i = 0; i < 9; i++)
    { // we need 9 bytes
      data[i] = ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    temp = (float)raw / 16.0;
    return 0;
  }
  return 2;
}

void ButtonInterrupt()
{
  static unsigned long lastInterruptTime = 0;
  static unsigned long currInterruptTime = millis();
  if (currInterruptTime - lastInterruptTime > BUTTON_DEBOUNCE_TIME)
  {
    buttonCounter += 1;
  }
}

void encoderInc()
{
  if ((millis() - time) > ENCODER_DEBOUNCE_TIME)
    encoderVal++;
  time = millis();
}

void encoderDec()
{
  if ((millis() - time) > ENCODER_DEBOUNCE_TIME)
    encoderVal--;
  time = millis();
}

void enterTemp()
{
  lcd.clear();

  encoderVal = temp;
  int begTemp = temp;
  while (buttonCounter == 0)
  {
    if (setTemp != encoderVal)
    {
      lcd.clear();
    }
    setTemp = encoderVal;
    lcd.setCursor(4, 0);
    lcd.print("Set temp:");
    lcd.setCursor(7, 1);
    lcd.print(setTemp);
    lcd.print((char)223);
    lcd.print("C");
    delay(100);
  }
  if (setTemp == begTemp)
  {
    tempAlarmFlag = false;
  }
  else if (setTemp < begTemp)
  {
    tempLoFLAG = true;
  }
  else
  {
    tempLoFLAG = false;
  }
}

void enterTime()
{
  lcd.clear();
  encoderVal = 0;
  int setSec = 0;
  while (buttonCounter == 1)
  {
    if (setTimer != encoderVal)
    {
      lcd.clear();
    }
    if (encoderVal < 0)
    {
      encoderVal = 0;
    }
    setTimer = encoderVal * 60;
    int sec = setTimer % 60;
    int min = setTimer / 60;
    lcd.setCursor(4, 0);
    lcd.print("Set Timer:");
    lcd.setCursor(7, 1);
    lcd.print(min);
    lcd.print(":");
    if (sec < 10)
    {
      lcd.print("0");
    }
    lcd.print(sec);
    delay(100);
  }

  int setMin = setTimer;
  encoderVal = 0;
  while (buttonCounter == 2)
  {

    if (setSec != encoderVal)
    {
      lcd.clear();
    }

    setSec = encoderVal;
    setTimer = setMin + setSec;
    if (setTimer < 0)
    {
      setTimer = 0;
    }
    int sec = setTimer % 60;
    int min = setTimer / 60;
    lcd.setCursor(4, 0);
    lcd.print("Set Timer:");
    lcd.setCursor(7, 1);
    lcd.print(min);
    lcd.print(":");
    if (sec < 10)
    {
      lcd.print("0");
    }
    lcd.print(sec);
    delay(100);
  }

  if (setTimer == 0)
  {
    timeAlarmFlag = false;
  }
}

void setup(void)
{
  temp = 0;
  ConversionStarted = false;
  Serial.begin(9600);
  pinMode(A0, INPUT_PULLUP); //button
  pinMode(2, INPUT_PULLUP);  //encoer
  pinMode(3, INPUT_PULLUP);  //encoder
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  attachInterrupt(0, encoderInc, LOW);
  attachInterrupt(1, encoderDec, LOW);
  attachPinChangeInterrupt(A0, ButtonInterrupt, RISING);

  lcd.init();          // Initialize I2C LCD module
  lcd.backlight();     // Turn backlight ON
  lcd.setCursor(3, 0); // Go to column 0, row 0
  lcd.print("Berga jest");
  lcd.setCursor(5, 1);
  lcd.print("SUPCIO");

  while (ReadTemp())
  {
    delay(1);
  }
  delay(250);
  enterTemp();
  enterTime();
  lcd.clear();
  startTime = millis();
}

void tempAlarmCheck()
{
  if (tempAlarmFlag)
  {
    if (tempLoFLAG)
    {
      if (temp <= setTemp)
      {
        tempAlarm = true;
      }
    }
    else
    {
      if (temp >= setTemp)
      {
        tempAlarm = true;
      }
    }
  }
}

void timeAlarmCheck()
{
  if (timeAlarmFlag)
  {
    if ((millis() > (startTime + (setTimer * 1000))) && timeAlarmFlag)
    {
      timeAlarm = true;
    }
  }
}
void ring()
{
  while (buttonCounter == 10)
  {
    digitalWrite(8, HIGH);
    delay(500);
    digitalWrite(8, LOW);
    delay(500);
  }
  while (true)
  {
    ReadTemp();
    lcd.setCursor(0, 0);
    lcd.print("TEMP");
    lcd.setCursor(0, 1);
    lcd.print(temp);
    delay(100);
  }
}

void alarmsCheck()
{
  if (!tempAlarmFlag && !timeAlarmFlag)
  {
    return;
  }
  else if (tempAlarmFlag && !timeAlarmFlag)
  {
    tempAlarmCheck();
    if (tempAlarm)
    {
      ring();
    }
  }
  else if (!tempAlarmFlag && timeAlarmFlag)
  {
    timeAlarmCheck();
    if (timeAlarm)
    {
      ring();
    }
  }
  else if (tempAlarmFlag && timeAlarmFlag)
  {
    static bool startedCountdown = false;
    tempAlarmCheck();
    if (tempAlarm)
    {
      if (!startedCountdown)
      {
        startTime = millis();
        startedCountdown = true;
      }
      timeAlarmCheck();
      if (timeAlarm)
      {
        ring();
      }
    }
  }
}

void loop(void)
{
  ReadTemp();
  alarmsCheck();
  {
    lcd.setCursor(0, 0);
    lcd.print("TEMP");
    lcd.setCursor(0, 1);
    lcd.print(temp);
    lcd.setCursor(12, 0);
    lcd.print("TIME");
    lcd.setCursor(12, 1);

    int sec;
    int min;
    unsigned long t;
    if (!timeAlarmFlag)
    {
      t = (millis()-startTime) / 1000;
    }
    else if (timeAlarmFlag && !tempAlarmFlag)
    {
      t = startTime / 1000 + setTimer - millis() / 1000;
    }
    else if (timeAlarmFlag && tempAlarmFlag)
    {
      if (!tempAlarm)
      {
        t = setTimer;
      }
      else
      {
        t = startTime / 1000 + setTimer - millis() / 1000;
      }
    }
    sec = t % 60;
    min = t / 60;
    lcd.print(min);
    lcd.print(":");
    if (sec < 10)
    {
      lcd.print("0");
    }
    lcd.print(sec);
  }
  buttonCounter = 10;
  // Serial.print("  ");
  // Serial.println(temp);
  delay(10);
}