About the CAN driver
--------------------

Principles
""""""""""

About CAN protocol
^^^^^^^^^^^^^^^^^^

CAN (Controller Area Network) is a widely used, robust, serial communication bus
which allows a set of MCU to communicate with each others. CAN buses are mostly
used in automotive systems. CAN is a message based protocol broadcasted on a
shared communication link. The CAN protocol handles priority based on the
message's identifier: the lower the CAN identifier, the higher the priority.
Each device can recieve all messages, or can use a hardware filter identifiers
to only recieve some messages.

Most of the time, devices use function-oriented protocols on the top of the CAN
protocol, such as CAN / ISO-TP / UDS for OBD-II.
