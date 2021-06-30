![https://www.microchip.com/](assets/microchip.png)
# SAMD21 ML Evaluation Kit Data Logger

## Overview
This repository contains firmware for streaming up to 6-axes IMU data over UART from one of the SAMD21 Machine Learning Evaluation Kits; the project can be configured to build firmware for both the [Bosch BMI160](https://www.microchip.com/developmenttools/ProductDetails/EV45Y33A) ([Mikroe IMU2 click board](https://www.mikroe.com/6dof-imu-2-click)) and [TDK ICM42688](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV18H79A) ([Mikroe IMU14 click board](https://www.mikroe.com/6dof-imu-14-click)) variants, streaming using one of several formats as described in the sections below.

| ![ml eval kits](assets/mlkits.png) |
| :--: |
| *Microchip Machine Learning Evaluation Kits* |

## Sensor Selection
Before building the firmware, select the appropriate MPLAB X Project Configuration for your sensor according to the table below.

| Sensor Type | MPLAB X Project Configuration Selection |
| --- | --- |
| [Bosch BMI160 IMU](https://www.microchip.com/developmenttools/ProductDetails/EV45Y33A) ([Mikroe IMU2 click board](https://www.mikroe.com/6dof-imu-2-click)) | `SAMD21_IOT_WG_BMI160` |
| [TDK ICM42688 IMU](https://www.microchip.com/DevelopmentTools/ProductDetails/PartNO/EV18H79A) ([Mikroe IMU14 click board](https://www.mikroe.com/6dof-imu-14-click)) | `SAMD21_IOT_WG_ICM42688` |

Project configuration can be set in the MPLAB X toolbar drop down menu as shown in the image below

| ![project config](assets/mplab-x-project-config.png) |
| :--: |
| *MPLAB X Project Configuration Selection* |

## Streaming Format Selection
To select the data streaming format, set the `DATA_STREAMER_FORMAT` macro in `firmware/src/app_config.h` to the appropriate value as explained in the table below.

| Streaming Format | app_config.h Configuration Value |
| --- | --- |
| ASCII text | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_ASCII` |
| [MPLAB Data Visualizer](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-data-visualizer) stream | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_MDV` |
| [SensiML Simple Stream](https://sensiml.com/documentation/simple-streaming-specification/introduction.html) | `#define DATA_STREAMER_FORMAT DATA_STREAMER_FORMAT_SMLSS` |

## Sensor Configuration Parameters
High level sensor parameters like sample rate and axes selection can be configured by modifying the macro values defined in `firmware/src/app_config.h`. See the inline comments for further description.

## Usage with the MPLAB Data Visualizer and Machine Learning Plugins
This project can be used to generate firmware for streaming data to the [MPLAB Data Visualizer plugin](https://www.microchip.com/en-us/development-tools-tools-and-software/embedded-software-center/mplab-data-visualizer) by setting the `DATA_STREAMER_FORMAT` macro as described in [the section above](#streaming-format-selection). Once the firmware is flashed, follow the steps below to set up Data Visualizer.

1. Connect the SAMD21 board to your computer, open up MPLAB X, and open the Data Visualizer plugin.
2. Click the *Load Workspace* button as highlighted in the image below. Select one of the workspace files included in this repository - located under the `mplab-dv-workspaces` folder - whose name most closely describes your sensor configuration; you can always modify the configuration once it is loaded if needed.

| ![load workspace](assets/load-ws.png) |
| :--: |
| *Loading the workspace file* |

3. Next, select the Serial/CDC Connection that corresponds to the SAMD21 board, adjust the baud rate to 115200, and click *Apply*.

| ![configure serial port](assets/serial-port-config.png) |
| :--: |
| *Configuring the SAMD21 serial port* |

4. Use the play button on the Serial/CDC Connection to connect to the serial port. Once the connection is made, the SAMD21 will be available for use with the variable streamer.

| ![start stream](assets/play-pause.png) |
| :--: |
| *Start Streaming* |

5. Switch to the *Variable Streamers* tab and select the aforementioned Serial/CDC Connection from the drop down menu as the input data source for the IMU variable streamer (see image below for reference); this will enable parsing of the SAMD21 data stream. You may delete or add variables here if your sensor configuration differs from the pre-loaded ones.

| ![data source](assets/data-source.png) |
| :--: |
| *Variable streamer data source selection* |

The IMU data should now be available in the time plot. Double click anywhere within the time plot to start/stop scrolling of the time axis

| ![time plot](assets/time-plot.png) |
| :--: |
| *Data Visualizer Time Plot* |

Visit the [Machine Learning Plugin page](https://microchipdeveloper.com/machine-learning:ml-plugin) to learn more about using the Data Visualizer plugin to export your data for machine learning applications.
