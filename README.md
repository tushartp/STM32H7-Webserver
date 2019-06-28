# STM32H7-Webserver


This is the secure webserver demo running on STM32H753I-EVAL board from STMicroelectronics

This demo uses webserver library from https://libwebsockets.org/

It contains the last known good commit(v3.1.0-174-g69fbc272) version of library from https://github.com/warmcat/libwebsockets

To compile the project please download free eclipse based IDE from ST's website https://www.st.com/en/development-tools/stm32cubeide.html

Prepare a SDcard and copy the contents of SDCARD folder on to the SDcard directly to have the webpages loaded from it. Insert the SD card in the uSD card slot on the Eval board. Compile the project and debug and run using ST-Link or J_link debugger.


This project has modified ethernetif.c file. The function low_level_input() was modified to have copy RX buffers to LWIP stack.
The option change be changed from lwipopts.h file with the macro ipconfigZERO_COPY_RX_DRIVER to 0 or 1.

The zero copy RX buffers does not work as of now as there may be issue with buffer overflow.
