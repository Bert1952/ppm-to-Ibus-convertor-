based on this tiny pcb https://nl.aliexpress.com/item/1005006468917096.html?spm=a2g0o.order_list.order_list_main.17.715679d2ExMyQZ&gatewayAdapt=glo2nld
on the backside of this pcb are 6 pads for program header.
Load the hex, set xtal freq to external (16 mhz). 
-small, stable signal. Input pin b0, output TX pin 115kb
Have fun!
PPM to iBus Converter

This project converts a standard PPM (trainer) signal from a legacy RC transmitter into a FlySky iBus serial data stream.

It was originally developed to continue using high-quality transmitters such as the Futaba FC-28 with modern flight controllers running Rotorflight, INAV, Betaflight, or any other firmware supporting the iBus protocol.

The converter is based on an Arduino Nano (ATmega328P, 16 MHz) and requires only a few connections.

Features
Converts up to 10 PPM channels to 14 iBus channels
Low latency
Lightweight AVR implementation (no Arduino libraries required)
Generates a standard FlySky iBus data stream
Suitable for modern flight controllers supporting iBus
Hardware

The project was developed using a standard Arduino Nano (ATmega328P, 16 MHz).

Connections
Arduino Nano	Function
D8 (PB0 / PCINT0)	PPM input
D1 (TX)	iBus output
D13	Status LED (optional)
D9	Debug output (optional)
5V	Power
GND	Ground

Only one input pin and one output pin are required.

Channel Mapping

The first ten PPM channels are copied directly into the iBus frame.

Channels 11–14 remain available for future expansion.

The converter performs no scaling or mixing. Incoming pulse widths are transmitted directly as iBus channel values, making it suitable for applications where accurate transmitter calibration is required.

Compatibility

Successfully tested with:

Futaba FC-28
FrSky RF module
Rotorflight
INAV
Betaflight

The generated iBus stream is compatible with standard FlySky iBus receivers and flight controllers.

Notes

The firmware is written in plain AVR C using:

Pin Change Interrupts for PPM decoding
Timer1 for pulse timing
Hardware UART for iBus transmission

No Arduino framework or external libraries are required.
