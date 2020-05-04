# Temperature Sensor with timer and Alarm

It's a home project - gift for a girlfriend. The device can measure temperature using DS18B20 sensor on a probe, show real time measurements on LCD display and set off alarm when needed. It uses barebones Atmega328 chip powered by 3 AA batteries. It can work in 4 modes.

# Work Modes
Temp | Timer | Result
-------|-------|-------
Not Set | Not Set | Temperature is printed in real time, timer counts from 0 upwards, no alarm is set
Not Set | Set | Temperature is printed in real time, timer counts down to 0, alarm when timer reaches 0
Set | Not Set | Temperature is printed in real time, timer counts from 0 upwards, alarm is set off when set temperature is reached
Set | Set | Temperature is printed in real time, timer shows set value and starts counting down when set temp is reached. Alarm is set off whem timer reaches 0

# Schematics
All schematics should be uploaded in [Eagle_files folder](https://github.com/KFrydryk/Temp-Alarm/tree/master/Eagle_files)