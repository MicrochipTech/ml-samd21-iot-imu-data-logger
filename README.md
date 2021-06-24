# SAMD21-IoT w/ IMU (SAMD21 ML Kit)

## Overview
This repository contains the firmware for streaming 6-axis IMU data over UART from the SAMD21 Machine Learning Kit with the [Bosch BMI160 IMU](https://www.microchip.com/developmenttools/ProductDetails/EV45Y33A) ([Mikroe IMU2 click board](https://www.mikroe.com/6dof-imu-2-click)) or with the [TDK ICM42688 IMU](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV18H79A) ([Mikroe IMU14 click board](https://www.mikroe.com/6dof-imu-14-click)). Several streaming formats are available as described [below](#streaming-format-selection).

## Sensor Selection
To select sensor type, select the appropriate MPLAB X Project Configuration in the toolbar as shown in the image below.

| ![load workspace](assets/mplab-x-project-config.png) |
| :--: |
| MPLAB X Project Configuration Selection |

The following table describes the different configurations.

| Sensor Type | MPLAB X Project Configuration Selection |
| --- | --- |
| [Bosch BMI160 IMU](https://www.microchip.com/developmenttools/ProductDetails/EV45Y33A) ([Mikroe IMU2 click board](https://www.mikroe.com/6dof-imu-2-click)) | `SAMD21_IOT_WG_BMI160` |
| [TDK ICM42688 IMU](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV18H79A) ([Mikroe IMU14 click board](https://www.mikroe.com/6dof-imu-14-click)) | `SAMD21_IOT_WG_ICM42688` |

## Streaming Format Selection
To select data streaming format, define `DATA_STREAMER_FORMAT` in `firmware/src/app_config.h` as explained in the table below.

| Streaming Format | app_config.h Configuration Value |
| --- | --- |
| ASCII text | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_ASCII` |
| [MPLAB Data Visualizer](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-data-visualizer) stream | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_MDV` |
| [SensiML Simple Stream](https://sensiml.com/documentation/simple-streaming-specification/introduction.html) | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_SMLSS` |

## Sensor Configuration Parameters
High level sensor parameters like sample rate and axes selection can be configured by modifying the values defined in `firmware/src/app_config.h`. See the inline comments for further description.

## Usage with MPLAB Data Visualizer and Machine Learning Plugins
This project can be used to generate firmware for streaming data to the [MPLAB Data Visualizer plugin](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-data-visualizer) by setting the `DATA_STREAMER_FORMAT` macro as described in [the section above](#streaming-format-selection).  Follow the steps below to configure Data Visualizer for use with your firmware.

With the SAMD21 board connected to your computer, open the Data Visualizer plugin and click the *Load Workspace* button as highlighted in the image below. Select one of the workspace files included in this repository - located under the `mplab-dv-workspaces` folder - whose name most closely describes your sensor configuration; you can always modify the configuration once it is loaded if needed.

| ![load workspace](assets/load-ws.png) |
| :--: |
| Loading the workspace file |

Next, select the Serial/CDC Connection that corresponds to the SAMD21 board, adjust the baud rate to 115200 and click *Apply*.

| ![configure serial port](assets/serial-port-config.png) |
| :--: |
| Configuring the SAMD21 serial port |

Use the play button on the Serial/CDC Connection to connect to the serial port. Once the connection is made, the SAMD21 will be available for use with the variable streamer.

| ![start stream](assets/play-pause.png) |
| :--: |
| Start Streaming |

Switch to the *Variable Streamers* tab and select the aforementioned Serial/CDC Connection as the input data source for the IMU variable streamer (see image below for reference); this will enable parsing of the SAMD21 data stream.

| ![data source](assets/data-source.png) |
| :--: |
| Variable streamer data source selection |

The IMU data should now be available in the time plot. Double click anywhere within the time plot to start/stop scrolling of the time axis

| ![time plot](assets/time-plot.png) |
| :--: |
| Data Visualizer Time Plot |

Visit the [Machine Learning Plugin page](https://microchipdeveloper.com/machine-learning:ml-plugin) to learn more about using the Data Visualizer plugin to export your data for machine learning applications.
