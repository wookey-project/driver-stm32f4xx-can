The CAN driver
==============

.. highlight:: c


This library is an implementation of a driver for the STM32F4 CANx device.

It provides an abstraction of the CAN device through an API, in order to mask
the hardware specificities, when one needs to interact with CAN buses.

The supported CAN standard versions are CAN 2.0 A and B, including:
 - standard CAN ID (11 bits) and extended CAN ID (29 bits)
 - the most common CAN bus speeds (100, 125, 250, 500, 1000 k bit/s)
 - the 7 CAN bus error codes.

This driver supports the various debugging modes of the STM32F4 CANx device,
including loop mode (CAN_MODE_LOOPBACK), receive only mode (CAN_MODE_SILENT) and
both (CAN_MODE_SILENT_LOOPBACK).


.. toctree::
   :caption: Table of Contents
   :name: mastertoc
   :maxdepth: 2

   About the CAN protocole <about>
   The drvCan API <api>
   FAQ <faq>
