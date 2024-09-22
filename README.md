# Overview
This project is a custom-built sous vide machine designed to provide precise temperature control for cooking. It features a robust PCB design and embedded programming tailored to manage the hardware components efficiently.

# Features
## Hardware
* Heating Element: 1500W, 230VAC to provide rapid and consistent heating.
* Temperature Sensor: DS18B20 digital sensor utilizing the 1-Wire protocol for accurate temperature measurements.
* Water Pump: Ensures even water circulation, providing a uniform temperature throughout the cooking process.
* OLED Display: Custom interface with menus for:
  * Temperature selection
  * Time selection
  * Pre-heating mode
  * Cooking mode
  * Ability to pause and modify temperature or time during cooking.
  * Buzzer Alarm: Alerts when the cooking cycle is complete or if there is an issue.
* Safety Features: Fuses and relays are implemented for basic electrical protection.
* HMI Interface: Two buttons with rotary functions for easy navigation and selection.
## Software
 * Microcontroller: STM32 with FreeRTOS for real-time task scheduling and efficient management of hardware components.
 * 1-Wire Protocol: Implemented for precise communication with the DS18B20 temperature sensor.
 * Custom Menu System: The OLED display is programmed to show temperature and time settings, cooking modes, and live cooking status.
 * Alarm and Notification System: A buzzer is programmed to alert users at key moments during the cooking process.

