# SAMD21-IoT w/ IMU (SAMD21 ML Kit)

## Overview
This repository contains the firmware for streaming 6-axis IMU data over UART to
[MPLAB Data Visualizer](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-data-visualizer)
from the SAMD21 Machine Learning Kit with the [Bosch BMI160 IMU](https://www.microchip.com/developmenttools/ProductDetails/EV45Y33A)
([Mikroe IMU2 click board](https://www.mikroe.com/6dof-imu-2-click)) or with the
[TDK ICM42688 IMU](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV18H79A)
([Mikroe IMU14 click board](https://www.mikroe.com/6dof-imu-14-click)).

## Firmware Configurations
The included project has 2 possible firmware 'modes' as well as the possible
selection of the IMU2 or IMU14 sensors, amounting to 4 possible firmware
configurations. The table below summarizes how to select each of these
configurations.

| Firmware Build Description | MPLAB X Project Configuration Selection | app_config.h Configuration Values | |
| --- | --- | --- | --- |
| Data logger - MPLAB DV format - Bosch IMU2 sensor | `SNSR_TYPE_BMI160` | `#define DATA_VISUALIZER_BUILD true` | `#define DATA_LOGGER_BUILD false` |
| Data logger - ASCII format - Bosch IMU2 sensor | `SNSR_TYPE_BMI160` | `#define DATA_VISUALIZER_BUILD false` | `#define DATA_LOGGER_BUILD true` |
| Data logger - MPLAB DV format - TDK IMU14 sensor | `SNSR_TYPE_ICM42688` | `#define DATA_VISUALIZER_BUILD true` | `#define DATA_LOGGER_BUILD false` |
| Data logger - ASCII format - TDK IMU14 sensor | `SNSR_TYPE_ICM42688` | `#define DATA_VISUALIZER_BUILD false` | `#define DATA_LOGGER_BUILD true` |