# Meditation Device - Arduino Prototype
> A work-in-progress meditation assistant that detaches you from your phone

## Table of Contents
* [General Information](#general-information)
* [Technology Used](#technology-used)
* [Project Status](#project-status)
* [Acknowledgements](#acknowledgements)

## General Information
This project was born out of the necessity to explore ways to merge the utility of a meditation practice, while detaching from the distractions of a mobile phone.

Using coloured light as an indicator of the passage of time, the device is intended to remove the fixation on numbers we have typically when using a clock, and completely remove the possibility of a mobile phone interjecting in your session.

This project is an exercise in learning about embedded software patterns, writing software for hardware that might come with manufacturing quirks, and converging these technologies into something that can stand on it's own in a household.

## Technology Used
The prototype is built on the Arduino framework implementing various modules for functionality.
* DS3231 Real-Time Clock for tracking time during meditation sessions
* RGB LED light strip
* 3 x TIP122 NPN transistors for powering the LED strip
* 3 x capacitive touch buttons for control and power

## Project Status
The project is currently in software development phase, with a focus on getting the controls to play nice with the sessions.

#### Work to follow
* Unit-testing
* User-testing
* Transfer to PCB
* Device design and prototyping

## Acknowledgements
[Arduino](https://www.arduino.cc/)

[Adafruit RTClib](https://adafruit.github.io/RTClib/html/class_time_span.html#details)
