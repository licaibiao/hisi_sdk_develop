#!/bin/sh              
                       
echo "run $0 begin!!!";

#I2C(¸´ÓÃÎªI2C)                                
himm 0x120F00E0 0x1; # 0£ºGPIO12_6  1£ºI2C_SDA 
himm 0x120F00E4 0x1; # 0£ºGPIO12_7  1£ºI2C_SCL

#I2S
himm 0x120F00A0 0x1; # 0: GPIO9_0   1: I2S0_BCLK_RX
himm 0x120F00A4 0x1; # 0: GPIO9_1   1: I2S0_WS_RX 
himm 0x120F00A8 0x1; # 0: GPIO9_2   1: I2S0_SD_RX

himm 0x120F00AC 0x2; # 00: GPIO9_3  01: I2S1_BCLK_RX  10:I2S2_MCLK 
himm 0x120F00B0 0x1; # 0: GPIO9_4   1: I2S1_WS_RX
himm 0x120F00B4 0x1; # 0: GPIO9_5   1: I2S1_SD_RX

himm 0x120F00B8 0x1; # 0: GPIO9_6   1: I2S2_BCLK_TX
himm 0x120F00BC 0x1; # 0: GPIO9_7   1: I2S2_WS_TX
himm 0x120F00C0 0x1; # 0: GPIO5_4   1: I2S2_SD_TX  

echo "run $0 end!!!"; 