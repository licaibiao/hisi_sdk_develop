#!/bin/sh

######### MISC QOS setting! ######

himm 0x1212007c 0x44443201  ## VGS-JPGD-IVE -TDE -AVC0- A7 - VO -VI
himm 0x12120080 0x26334444  ## GSF-DDRT-AVC1-VPSS-VOIE-JPGE-AIO-MDU
himm 0x12120084 0x66666426  ## ###-DMAm0-DMAm1-FMC-USB2-CIPHER-SCD-SATA

#######VIVO 总线优先级 7优先级最大###########
himm 0x12120094 0x65   ###【2:0】VI   【6:4】VO
 
###############################
## mddrc0 pri&timeout setting #
###############################
himm  0x12110020  0x00000001  # AXI_ACTION[19:8]:wr_rcv_mode=0,12ports 

himm  0x12110200  0x00370000  # ports0 选择随路QOS模式
himm  0x12110210  0x00370000  # ports1
himm  0x1211021c  0x08300830  # ports1读写自适应优先级
himm  0x12110220  0x00370000  # ports2 
himm  0x1211022c  0x08300830  # port2读写自适应优先级
himm  0x12110230  0x00370000  # ports3 
himm  0x1211023c  0x08300830  # port3读写自适应优先级
himm  0x12110240  0x00370000  # ports4 
himm  0x12110250  0x00370000  # ports5 
himm  0x12110260  0x00370000  # ports6 
himm  0x12110270  0x00370000  # ports7 
##DDRC AXI pri ports0 - 7
############## WR pri ##############
himm  0x12110204  0x76543210  # ports0         
himm  0x12110214  0x76543210  # ports1         
himm  0x12110224  0x76543210  # ports2
himm  0x12110234  0x76543210  # ports3
himm  0x12110244  0x76543210  # ports4
himm  0x12110254  0x76543210  # ports5
himm  0x12110264  0x76543210  # ports6   
himm  0x12110274  0x76543210  # ports7
############## RD pri ##############
himm  0x12110208  0x76543210  # ports0         
himm  0x12110218  0x76543210  # ports1         
himm  0x12110228  0x76543210  # ports2
himm  0x12110238  0x76543210  # ports3
himm  0x12110248  0x76543210  # ports4
himm  0x12110258  0x76543210  # ports5
himm  0x12110268  0x76543210  # ports6
himm  0x12110278  0x76543210  # ports7
##############  qosbuf #############
himm  0x12114000  0x00000002   #qosb_push_ctrl
himm  0x12114004  0x000007F1   #cycle
himm  0x1211410c  0x0000000a   #qosb_dmc_lvl
himm  0x12114110  0x0000000a   #qosb_dmc_lvl
himm  0x1211408c  0xb3032010   #qosb_wbuf_ctrl
himm  0x12114090  0xb3032010   #qosb_wbuf_ctrl
himm  0x121140f4  0x00000033   #row-hit enable
himm  0x121140ec  0x00000044   #row-hit 
himm  0x121140f0  0x00003333   #row-hit
himm  0x121141f4  0x00000000   #qosb_wbuf_pri_ctrl

himm  0x121141f0  0x00000001   #enable qosbuf timeout,through prilvl to remap timeout level
############## WR timeout ###########
himm  0x1211409c  0x00000010  # wr_tout3 ~wr_tout0         
himm  0x121140a0  0x00000000  # wr_tout7 ~wr_tout4         
himm  0x121140a4  0x00000000  # wr_tout11~wr_tout8
himm  0x121140a8  0x00000000  # wr_tout15~wr_tout12

############## RD timeout ###########
himm  0x121140ac  0x00000010  # rd_tout3 ~rd_tout0          
himm  0x121140b0  0x00000000  # rd_tout7 ~rd_tout4          
himm  0x121140b4  0x00000000  # rd_tout11~rd_tout8 
himm  0x121140b8  0x00000000  # rd_tout15~rd_tout12

himm  0x121141f8  0x00800002  # qosb_rhit_ctrl,open_window=128,close_window=2

