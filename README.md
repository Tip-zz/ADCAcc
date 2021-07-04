# ADCAcc
Nov-2020  TEP

No warranty made or implied.
Free for all use.
No attribution required.

This project has a three axis accelerometer and three ADC inputs.
Originally built to diagnose unstable Mazda voltage regulator that
seemed to be sensitive to acceleration.

Includes Arduino Nano SBC and Adafruit LIS3DH Triple-Axis Accelerometer.

Analog inputs 0..25.55V, 10 bit.
Accelerometer 3 axis, 12 bit, +/-2g, 4g, 8g, or 16g.

Output on USB virtual serial port at 115200 Baud.

ADCAcc v0.7
Sample rate (Samp/s):   20.00
(Actual sample rate):   20.03
Voltage full scale (V): 25.57
Accel full scale (G):   8
Accel X zero:           0
Accel Y zero:           0
Accel Z zero:           0
>?

<Space> - Toggle Run/Stop.
R - Run.
S - Stop.
Z - Zero Accel channels.
U - Unzero Accel channels.
F - Set Sampling rate (Samples/s).
2 - Set Accel 2G full scale.
4 - Set Accel 4G full scale.
8 - Set Accel 8G full scale.
6 - Set Accel 16G full scale.
V - Set Voltage full scale.
P - Display Parameters.
D - Restore Default parameters.
E - EEPROM dump.
H - Display this help menu.
>r
V1      V1      V3      X       Y       Z
0.00    0.00    0.00    0.01    0.00    0.96
0.00    0.00    0.00    0.01    0.00    0.97
0.00    0.00    0.00    0.01    0.01    0.96
0.00    0.00    0.00    0.00    0.01    0.96
0.00    0.00    0.00    0.00    0.01    0.96
0.00    0.00    0.00    0.00    0.00    0.96
s
>
