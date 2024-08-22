Overview
========
The project contains a software implementation of EVSE-SIG-BRD features in EVSE mode. 
 - Generate J1772 Control Pilot PWM signal
 - Measure PWM level
 - Measure Proximity pilot level
 - Measure GFCI event
 - Drive external relay
 - Serial communication through host connector
Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Host controller board e.g., MIMXRT1064-EVK, MIMX1060-EVK(B), i.MX93-EVK, i.MX8MNano-EVK etc.
- Personal Computer

Board settings
==============
* Jumper setting of the board for Power supply 
 - Place J2 to 1,2 and J3 to 1,2 positions if powered by external 5V DC adapter connected to J1
 - Keep default J2 to 2,3 and J3 to 2,3 positions if powered by above mentioned host controller boards Arduino Uno/EXP CN connectors

Prepare to start
================
1. Connect a USB-to-serial TTL adapter between the PC host and the board connector J46 pins 2(TXD), 3(RXD) and 4(Ground).
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Power on the board either by an external 5V DC adapter connected to J1 or by the above mentioned host controller boards Arduino Uno/EXP CN connectors.

Running the software
====================
When the software runs successfully, the LEDS D18, D19 blink at the rate of 2Hz.
When the software runs successfully, the following message is displayed on the terminal:
Initializing EVSE-SIG-BRD for EVSE..
Initialized control pilot PWM to Low state.
Initialized Proximity Pilot.
Initialized GFCI.
Initialized Relay.
Initialized board LEDs.
Initialization done.
Entering main loop.

On pressing SW1 on the board will display the below message on the terminal:
Emergency stop event occurred - push button pressed.
Emergency stop event restored - push button released.

