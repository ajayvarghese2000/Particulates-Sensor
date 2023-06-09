<p align="center">
	<a href="https://github.com/lboroWMEME-TeamProject/CCC-ProjectDocs"><img src="https://i.imgur.com/VwT4NrJ.png" width=650></a>
	<p align="center"> This repository is part of  a collection for the 21WSD001 Team Project. 
	All other repositories can be access below using the buttons</p>
</p>

<p align="center">
	<a href="https://github.com/lboroWMEME-TeamProject/CCC-ProjectDocs"><img src="https://i.imgur.com/rBaZyub.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Dashboard"><img src="https://i.imgur.com/fz7rgd9.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Cloud-Server"><img src="https://i.imgur.com/bsimXcV.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Drone-Firmware"><img src="https://i.imgur.com/yKFokIL.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Simulated-Drone"><img src="https://i.imgur.com/WMOZbrf.png" alt="drawing" height = 33/></a>
</p>


<p align="center">
	Below you can find buttons that link you to the repositories that host the code for the module itself. These can also be found linked in the collection repository: <a href="https://github.com/lboroWMEME-TeamProject/Drone-Firmware">Drone Firmware</a>. 
</p>


<p align="center">
	<a href="https://github.com/lboroWMEME-TeamProject/Main-Pi"><img src="https://i.imgur.com/4knNDhv.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/EnviroSensor"><img src="https://i.imgur.com/lcYUZBw.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Geiger-Counter"><img src="https://i.imgur.com/ecniGik.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/Thermal-Camera"><img src="https://i.imgur.com/kuoiBTc.png" alt="drawing" height = 33/></a> 
	<a href="https://github.com/lboroWMEME-TeamProject/ai-cam"><img src="https://i.imgur.com/30bEKvR.png" alt="drawing" height = 33/></a>
</p>

------------

# Particulates-Sensor

This repository contains code that uses the Raspberry Pi Pico to interface with the PMS5003, particulates matter sensor, collect its data and send it back to the Main Pi over I2C.

------------

## Table of Contents

- [Subsystem Overview](#Subsystem-Overview)
    - [Wiring Diagram](Wiring-Diagram)
- [Code Overview](#Code-Overview)
	- [Registers](#Registers)
	- [Event Loops](#Event-Loops)
- [Test Plan](#Test-Plan)
- [Installation](#Installation)
	- [Prerequisites](#Prerequisites)
- [Deployment](#Deployment)

------------

## Subsystem Overview

The PMS5003 sensor communicates over UART and sends a data packet of 32 bytes that needs to be decoded. We do not need all that data in this packet so we only extract data bytes for the 3 different levels of particulates we're monitoring, PM1, PM2.5 and PM10. Once the extraction is done the data is written to the respective registers within memory.


**Subsystem Diagram :**

<p align="center">
	<img src="https://i.imgur.com/1waewGY.jpg" alt="High Level Diagram"/>
</p>



### Wiring Diagram

<p align="center">
	<img src="https://i.imgur.com/Hmv5sFq.jpg" alt="High Level Diagram"/>
</p>

------------

## Code Overview

The Raspberry Pico acts as an I2C Slave when connected, this slave has the default address **0x"D**. It has 4 16 bit registers in memory to store and access the data from the geiger counter.

## Registers

### 0x00

The first register at location **0x00** holds the device address of the connected I2C Slave

This is sent as 2 I2C Data packets when accessed however the first 8 bits can be ignored as I2C slaves can only take 8 bit address values.
<p align="center">
	<table align="center">
		<thead align="center">
			<tr align="center">
				<th align="center">Bit 15</th>
				<th align="center">Bit 14</th>
				<th align="center">Bit 13</th>
				<th align="center">Bit 12</th>
				<th align="center">Bit 11</th>
				<th align="center">Bit 10</th>
				<th align="center">Bit 09</th>
				<th align="center">Bit 08</th>
			</tr>
		</thead>
		<tbody align="center">
			<tr align="center">
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
				<td align="center">Reserved</td>
			</tr>
		</tbody>
		<thead align="center">
			<tr>
				<th align="center">Bit 07</th>
				<th align="center">Bit 06</th>
				<th align="center">Bit 05</th>
				<th align="center">Bit 04</th>
				<th align="center">Bit 03</th>
				<th align="center">Bit 02</th>
				<th align="center">Bit 01</th>
				<th align="center">Bit 00</th>
			</tr>
		</thead>
		<tbody align="center">
			<tr>
				<td align="center">DEVICE_ADDRESS.07</td>
				<td align="center">DEVICE_ADDRESS.06</td>
				<td align="center">DEVICE_ADDRESS.05</td>
				<td align="center">DEVICE_ADDRESS.04</td>
				<td align="center">DEVICE_ADDRESS.03</td>
				<td align="center">DEVICE_ADDRESS.02</td>
				<td align="center">DEVICE_ADDRESS.01</td>
				<td align="center">DEVICE_ADDRESS.00</td>
			</tr>
		</tbody>
	</table>
</p>

### 0x01

The second register at location **0x01** holds the PM1 particles detected from the PMS5003 Sensor

This is sent as 2 I2C Data packets when accessed the first data packet contains bits 15:8 of the PM1 particles and the second data packet contains bits 7:0 of the PM1 particles value.

When the data is received you must reconstruct the data packets to get the actual reading of the PMS5003 Sensor.

`PMS5003_Example.py` contains code that allows you to reconstruct the bits as well as initialise the I2C PMS5003 Sensor.

<p align="center">
	<table align="center">
		<thead>
			<tr>
				<th align="center">Bit 15</th>
				<th align="center">Bit 14</th>
				<th align="center">Bit 13</th>
				<th align="center">Bit 12</th>
				<th align="center">Bit 11</th>
				<th align="center">Bit 10</th>
				<th align="center">Bit 09</th>
				<th align="center">Bit 08</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM1.15</td>
				<td align="center">PM1.14</td>
				<td align="center">PM1.13</td>
				<td align="center">PM1.12</td>
				<td align="center">PM1.11</td>
				<td align="center">PM1.10</td>
				<td align="center">PM1.09</td>
				<td align="center">PM1.08</td>
			</tr>
		</tbody>
		<thead>
			<tr>
				<th align="center">Bit 07</th>
				<th align="center">Bit 06</th>
				<th align="center">Bit 05</th>
				<th align="center">Bit 04</th>
				<th align="center">Bit 03</th>
				<th align="center">Bit 02</th>
				<th align="center">Bit 01</th>
				<th align="center">Bit 00</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM1.07</td>
				<td align="center">PM1.06</td>
				<td align="center">PM1.05</td>
				<td align="center">PM1.04</td>
				<td align="center">PM1.03</td>
				<td align="center">PM1.02</td>
				<td align="center">PM1.01</td>
				<td align="center">PM1.00</td>
			</tr>
		</tbody>
	</table>
</p>

### 0x02

The third register at location **0x02** holds the PM2.5 particles detected from the PMS5003 Sensor

This is sent as 2 I2C Data packets when accessed the first data packet contains bits 15:8 of the PM2.5 particles and the second data packet contains bits 7:0 of the PM2.5 particles value.

When the data is received you must reconstruct the data packets to get the actual reading of the PMS5003 Sensor.

`PMS5003_Example.py` contains code that allows you to reconstruct the bits as well as initialise the I2C PMS5003 Sensor.

<p align="center">
	<table align="center">
		<thead>
			<tr>
				<th align="center">Bit 15</th>
				<th align="center">Bit 14</th>
				<th align="center">Bit 13</th>
				<th align="center">Bit 12</th>
				<th align="center">Bit 11</th>
				<th align="center">Bit 10</th>
				<th align="center">Bit 09</th>
				<th align="center">Bit 08</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM2.5.15</td>
				<td align="center">PM2.5.14</td>
				<td align="center">PM2.5.13</td>
				<td align="center">PM2.5.12</td>
				<td align="center">PM2.5.11</td>
				<td align="center">PM2.5.10</td>
				<td align="center">PM2.5.09</td>
				<td align="center">PM2.5.08</td>
			</tr>
		</tbody>
		<thead>
			<tr>
				<th align="center">Bit 07</th>
				<th align="center">Bit 06</th>
				<th align="center">Bit 05</th>
				<th align="center">Bit 04</th>
				<th align="center">Bit 03</th>
				<th align="center">Bit 02</th>
				<th align="center">Bit 01</th>
				<th align="center">Bit 00</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM2.5.07</td>
				<td align="center">PM2.5.06</td>
				<td align="center">PM2.5.05</td>
				<td align="center">PM2.5.04</td>
				<td align="center">PM2.5.03</td>
				<td align="center">PM2.5.02</td>
				<td align="center">PM2.5.01</td>
				<td align="center">PM2.5.00</td>
			</tr>
		</tbody>
	</table>
</p>

### 0x03

The third register at location **0x03** holds the PM10 particles detected from the PMS5003 Sensor

This is sent as 2 I2C Data packets when accessed the first data packet contains bits 15:8 of the PM10 particles and the second data packet contains bits 7:0 of the PM10 particles value.

When the data is received you must reconstruct the data packets to get the actual reading of the PMS5003 Sensor.

`PMS5003_Example.py` contains code that allows you to reconstruct the bits as well as initialise the I2C PMS5003 Sensor.

<p align="center">
	<table align="center">
		<thead>
			<tr>
				<th align="center">Bit 15</th>
				<th align="center">Bit 14</th>
				<th align="center">Bit 13</th>
				<th align="center">Bit 12</th>
				<th align="center">Bit 11</th>
				<th align="center">Bit 10</th>
				<th align="center">Bit 09</th>
				<th align="center">Bit 08</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM10.15</td>
				<td align="center">PM10.14</td>
				<td align="center">PM10.13</td>
				<td align="center">PM10.12</td>
				<td align="center">PM10.11</td>
				<td align="center">PM10.10</td>
				<td align="center">PM10.09</td>
				<td align="center">PM10.08</td>
			</tr>
		</tbody>
		<thead>
			<tr>
				<th align="center">Bit 07</th>
				<th align="center">Bit 06</th>
				<th align="center">Bit 05</th>
				<th align="center">Bit 04</th>
				<th align="center">Bit 03</th>
				<th align="center">Bit 02</th>
				<th align="center">Bit 01</th>
				<th align="center">Bit 00</th>
			</tr>
		</thead>
		<tbody>
			<tr>
				<td align="center">PM10.07</td>
				<td align="center">PM10.06</td>
				<td align="center">PM10.05</td>
				<td align="center">PM10.04</td>
				<td align="center">PM10.03</td>
				<td align="center">PM10.02</td>
				<td align="center">PM10.01</td>
				<td align="center">PM10.00</td>
			</tr>
		</tbody>
	</table>
</p>


## Event Loops

The Program is dependent on interrupts and the main loop does nothing after setting up the peripherals.

There are two event loops in the program, the UART Loop and the I2C Loop.

The I2C Loop takes priority over the UART Loop.

### UART Loop

<p align="center">
	<img src="https://i.imgur.com/RwyUM4n.jpg" alt="drawing"/>
</p>

### I2C Loop

<p align="center">
	<img src="https://i.imgur.com/PZDLVok.jpg" alt="drawing"/>
</p>



------------

## Test Plan

<div align="center">

|Objective|Testing Strategy|Expected Output|Current Output|Pass/Fail|
|--|--|--|--|:--:|
|PMS53003 Detects changes in the air quality and outputs the correct data to the UART console|Run code to get the raw data from the PMS sensor onto a serial monitor; once the particulates data has settled spray some deodorant and monitor the particulates per m³ data.|Once turned on the sensor output should settle to a consistent value. Then when the deodorant is sprayed the number of particulates per meter³ should increase|Once switched on the value does settle to a consistent value and when the deodorant is sprayed the value does increase.|:heavy_check_mark:|
|The PMS5003 should send its data to the Pi Pico and the data should be received successfully by the Pico|Connect the PMS sensor to the Pico's UART pins and run code on the Pico to output the data to the serial monitor.|The data from the PMS sensor should show up on screen and it should pass all CRC checks|The data is sent and received correctly|:heavy_check_mark:|
|Raspberry Pi Pico sends CPM to the Raspberry Pi 4 (master).| Obtain values from I2C bus and check the data correlates with the data collected from previous two tests.|Correct data should display in C program window.|Correct data is displayed.|:heavy_check_mark:|
|Correct data is being sent to GUI.|Observe data is bring printed in GUI. Ensure data corresponds with previously collected data to calibrate against.|Data on GUI should appear and correlate with expected results.|Correct data is displayed.|:heavy_check_mark:|
</div>

------------

## Installation

This program is written in C for the Raspberry Pico. You can compile the code using the supplied makefile however, you must make sure you have the required toolchains and environment variables installed don your local machine.

### Prerequisites

- [Pico C/C++ SKD](https://github.com/raspberrypi/pico-sdk)
- [GNU Embedded Toolchain for Arm](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain)
- [CMake](https://cmake.org/)
- [VScode](https://code.visualstudio.com/)
- [CMake Tools for VScode](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
- [Python 3.x](https://www.python.org/)
- [Git](https://git-scm.com/)

if you're on windows you will also need:

- [MinGW-w64](https://www.mingw-w64.org/)


Once all the required tool chains are installed and configured open the repository in VScode.

A full guide on hot to get the tool chains set up can be found on the [Getting Started With Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) documentation written by Raspberry Pi Foundation.

Once opened in VScode CMake Tools will automatically configure the project.

You can edit the `CMakeLists.txt` files to add or remove compilation files/libraries.

Once you have made the changes you want, you can compile using CMake. It will create a `.uf2` file with the specified settings in the `build/` directory that you can use and upload to the Pico.


------------

## Deployment

Once you have compiled the code you will end up with a `.uf2` file. This contains the binary code that needs to be uploaded to the Raspberry Pico. 

This repository contains a `.uf2` file with the default settings in the `build/` directory that you can use.

If you need to change options you need to recompile the code, refer to the [Installation](#installation) section for instructions on how to do that.

**Step 1:** Plug in the Raspberry Pico whilst holding down the BOOTSEL button.

**Step 2:** Navigate to the Pico file directory

**Step 3:** Copy over the `.uf2` file from the `build/` directory. 

Now once you have connected the device as the [Wiring Diagram](#wiring-diagram) your I2C device should be accessible and outputting data when requested.

------------

