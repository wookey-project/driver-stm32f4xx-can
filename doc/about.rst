About the CAN driver
--------------------

Principles
""""""""""

The CAN protocol
^^^^^^^^^^^^^^^^

CAN (Controller Area Network) is a widely used, robust, serial communication bus
which allows a set of MCU to communicate with each others. CAN buses are mostly
used in automotive systems.

The CAN is a message-based protocol, where each message is broadcasted on a
shared communication link, composed of two wires. The CAN protocol handles
priority based on the message's identifier: the lower the CAN identifier, the
higher the priority. Each device can recieve all messages, or can use a hardware
filter on the CAN indentifier to only recieve some messages.

A CAN frame is limited to a payload of 8 bytes only and is best suited for short
real-time messages. Often, devices use function-oriented protocols on the top of
the CAN protocol, to provide more flexibility, such as CAN / ISO-TP / UDS for
the diagnostic services in OBD-II.
