# Simple UART Config Protocol

For the configuration of the device, there is a simple protocol using Asynchronous Serial, in this document,
you will find what is needed to configure the device with the serial, either manually or if you want to make a program
to do it.

## Specification

The protocol uses a very simple compact text format to set values.

Each message starts with a 2 byte break consisting of 0xAAAA and then a message length byte 3-255 is the valid range. (3
is the minimum number of characters to complete a theoretical mimimal package)

Refer to the table below for example of message construction.

| Break0 | Break1 | Length | Message String (ASCII) |
|--------|--------|--------|------------------------|
| 0xAA   | 0xAA   | 0xE    | TARGET_TEMP=10         |

The message string can contain multiple values using `;` as a seperator as shown below:

`TARGET_TEMP=10;TARGET_HUMIDITY=40`