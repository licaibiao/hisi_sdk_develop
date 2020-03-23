#!/bin/sh              
                       
echo "run $0 begin!!!"; 

#VGA
himm 0x120F0098 0x1;   # 0: GPIO11_6  1: VGA_HS
himm 0x120F009C 0x1;   # 0: GPIO11_3  1: VGA_VS

#HDMI
himm 0x120F0174 0x1;   # 0: GPIO13_4  1:HDMI_HOTPLUG
himm 0x120F0178 0x1;   # 0: GPIO13_5  1:HDMI_CEC
himm 0x120F017C 0x1;   # 0: GPIO13_6  1:HDMI_SDA
himm 0x120F0180 0x1;   # 0: GPIO13_7  1:HDMI_SCL

#SPI
himm 0x120F00C4 0x1;   # 00:TEST_CLK 01:SPI_SCLK 10:GPIO5_0
himm 0x120F00C8 0x1;   # 0: GPIO5_1   1:SPI_SDO
himm 0x120F00CC 0x1;   # 0: GPIO5_2   1:SPI_SDI
himm 0x120F00D0 0x1;   # 0: GPIO5_3   1:SPI_CSN0 
himm 0x120F00D4 0x1;   # 0: GPIO5_4   1:SPI_CSN1

echo "run $0 end!!!"; 