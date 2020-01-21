#ifndef HST_CHN_CONFIG_H
#define HST_CHN_CONFIG_H
#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
	DEV_VERSION_NULL = 0,
	DEV_VERSION_BT10 = 10,
	DEV_VERSION_BT20 = 20,
}DEV_VERSION_BT_E;

typedef enum
{
    AHD_CH1_STOR_V = 0x00,
    AHD_CH2_STOR_V = 0x01,
    AHD_CH3_STOR_V = 0x02,
    AHD_CH4_STOR_V = 0x03,
    AHD_CH5_STOR_V = 0x04,
    AHD_CH6_STOR_V = 0x05,
    AHD_CH7_STOR_V = 0x06,
    AHD_CH8_STOR_V = 0x07,

    AHD_CH1_SEND_V = 0x08,
    AHD_CH2_SEND_V = 0x09,
    AHD_CH3_SEND_V = 0x0A,
    AHD_CH4_SEND_V = 0x0B,
    AHD_CH5_SEND_V = 0x0C,
    AHD_CH6_SEND_V = 0x0D,
    AHD_CH7_SEND_V = 0x0E,
    AHD_CH8_SEND_V = 0x0F,

    AHD_CH1_SNAP = 0x10,
    AHD_CH2_SNAP = 0x11,
    AHD_CH3_SNAP = 0x12,
    AHD_CH4_SNAP = 0x13,
    AHD_CH5_SNAP = 0x14,
    AHD_CH6_SNAP = 0x15,
    AHD_CH7_SNAP = 0x16,
    AHD_CH8_SNAP = 0x17,

    IPC_CH1_STOR_V = 0x18,
    IPC_CH2_STOR_V = 0x19,
    IPC_CH3_STOR_V = 0x1A,
    IPC_CH4_STOR_V = 0x1B,
    IPC_CH5_STOR_V = 0x1C,
    IPC_CH6_STOR_V = 0x1D,
    IPC_CH7_STOR_V = 0x1E,
    IPC_CH8_STOR_V = 0x1F,

    IPC_CH1_SEND_V = 0x20,
    IPC_CH2_SEND_V = 0x21,
    IPC_CH3_SEND_V = 0x22,
    IPC_CH4_SEND_V = 0x23,
    IPC_CH5_SEND_V = 0x24,
    IPC_CH6_SEND_V = 0x25,
    IPC_CH7_SEND_V = 0x26,
    IPC_CH8_SEND_V = 0x27,

    IPC_CH1_SNAP = 0x28,
    IPC_CH2_SNAP = 0x29,
    IPC_CH3_SNAP = 0x2A,
    IPC_CH4_SNAP = 0x2B,
    IPC_CH5_SNAP = 0x2C,
    IPC_CH6_SNAP = 0x2D,
    IPC_CH7_SNAP = 0x2E,
    IPC_CH8_SNAP = 0x2F,

    VIDEO_CHN_INVALID = 0xFF,
}VIDEO_CHN_E;

typedef enum
{
    AHD_CH1_STOR_A = 0x00,
    AHD_CH2_STOR_A = 0x01,
    AHD_CH3_STOR_A = 0x02,
    AHD_CH4_STOR_A = 0x03,
    AHD_CH5_STOR_A = 0x04,
    AHD_CH6_STOR_A = 0x05,
    AHD_CH7_STOR_A = 0x06,
    AHD_CH8_STOR_A = 0x07,

    AHD_CH1_SEND_A = 0x08,
    AHD_CH2_SEND_A = 0x09,
    AHD_CH3_SEND_A = 0x0A,
    AHD_CH4_SEND_A = 0x0B,
    AHD_CH5_SEND_A = 0x0C,
    AHD_CH6_SEND_A = 0x0D,
    AHD_CH7_SEND_A = 0x0E,
    AHD_CH8_SEND_A = 0x0F,

    AHD_CH1_TAPE = 0x10,
    AHD_CH2_TAPE = 0x11,
    AHD_CH3_TAPE = 0x12,
    AHD_CH4_TAPE = 0x13,
    AHD_CH5_TAPE = 0x14,
    AHD_CH6_TAPE = 0x15,
    AHD_CH7_TAPE = 0x16,
    AHD_CH8_TAPE = 0x17,

    IPC_CH1_STOR_A = 0x18,
    IPC_CH2_STOR_A = 0x19,
    IPC_CH3_STOR_A = 0x1A,
    IPC_CH4_STOR_A = 0x1B,
    IPC_CH5_STOR_A = 0x1C,
    IPC_CH6_STOR_A = 0x1D,
    IPC_CH7_STOR_A = 0x1E,
    IPC_CH8_STOR_A = 0x1F,

    IPC_CH1_SEND_A = 0x20,
    IPC_CH2_SEND_A = 0x21,
    IPC_CH3_SEND_A = 0x22,
    IPC_CH4_SEND_A = 0x23,
    IPC_CH5_SEND_A = 0x24,
    IPC_CH6_SEND_A = 0x25,
    IPC_CH7_SEND_A = 0x26,
    IPC_CH8_SEND_A = 0x27,

    IPC_CH1_TAPE = 0x28,
    IPC_CH2_TAPE = 0x29,
    IPC_CH3_TAPE = 0x2A,
    IPC_CH4_TAPE = 0x2B,
    IPC_CH5_TAPE = 0x2C,
    IPC_CH6_TAPE = 0x2D,
    IPC_CH7_TAPE = 0x2E,
    IPC_CH8_TAPE = 0x2F,

    MIC_CH1_TALK = 0x30,/*¶Ô½²±àÂëÍ¨µÀ1 2018-06-28*/

    AUDIO_CHN_INVALID = 0xFF,
}AUDIO_CHN_E;
#ifdef __cplusplus
}
#endif

#endif
