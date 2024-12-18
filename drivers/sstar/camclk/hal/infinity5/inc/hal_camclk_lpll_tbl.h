/* Copyright (c) 2018-2019 Sigmastar Technology Corp.
 All rights reserved.

 Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
Sigmastar Technology Corp. and be kept in strict confidence
(Sigmastar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of Sigmastar Confidential
Information is unlawful and strictly prohibited. Sigmastar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/

#ifndef _CAMCLK_LPLL_TBL_H_
#define _CAMCLK_LPLL_TBL_H_
#include "drv_camclk_DataType.h"
//-------------------------------------------------------------------------------------------------
//  Defines & Macro
//-------------------------------------------------------------------------------------------------

#define HAL_CAMCLK_LPLL_REG_NUM    6

#define DATA_LANE_9MHZ     (9000000)
#define DATA_LANE_9_5MHZ   (9500000)
#define DATA_LANE_12_5MHZ  (12500000)
#define DATA_LANE_25MHZ    (25000000)
#define DATA_LANE_50MHZ    (50000000)
#define DATA_LANE_100MHZ   (100000000)
#define DATA_LANE_187_5MHZ (187500000)
#define DATA_LANE_200MHZ   (200000000)
#define DATA_LANE_400MHZ   (400000000)
#define DATA_LANE_800MHZ   (800000000)
#define DATA_LANE_1500MHZ  (1500000000)

#define IS_DATA_LANE_LESS_100M(bps)            ( bps <= DATA_LANE_100MHZ )
#define IS_DATA_LANE_BPS_100M_TO_200M(bps)     ( (bps > DATA_LANE_100MHZ) && (bps <= DATA_LANE_200MHZ ) )
#define IS_DATA_LANE_BPS_200M_TO_400M(bps)     ( (bps > DATA_LANE_200MHZ) && (bps <= DATA_LANE_400MHZ ) )
#define IS_DATA_LANE_BPS_400M_TO_800M(bps)     ( (bps > DATA_LANE_400MHZ) && (bps <= DATA_LANE_800MHZ ) )
#define IS_DATA_LANE_BPS_800M_TO_15000M(bps)   ( (bps > DATA_LANE_800MHZ) && (bps <= DATA_LANE_1500MHZ ) )

#define IS_DATA_LANE_LESS_9M(bps)              (  bps < DATA_LANE_9MHZ )
#define IS_DATA_LANE_BPS_9M_TO_9_5M(bps)       ( (bps >= DATA_LANE_9MHZ) && (bps < DATA_LANE_9_5MHZ ) )
#define IS_DATA_LANE_BPS_12_5M_TO_25M(bps)     ( (bps > DATA_LANE_12_5MHZ) && (bps <= DATA_LANE_25MHZ ) )
#define IS_DATA_LANE_BPS_25M_TO_50M(bps)       ( (bps > DATA_LANE_25MHZ) && (bps <= DATA_LANE_50MHZ ) )
#define IS_DATA_LANE_BPS_50M_TO_100M(bps)      ( (bps > DATA_LANE_50MHZ) && (bps <= DATA_LANE_100MHZ ) )
#define REG_LPLL_BASE           0x103300UL
#define REG_LPLL_48_L     (REG_LPLL_BASE + 0x90)
#define REG_LPLL_48_H     (REG_LPLL_BASE + 0x91)
#define REG_LPLL_49_L     (REG_LPLL_BASE + 0x92)
#define REG_LPLL_49_H     (REG_LPLL_BASE + 0x93)

//-------------------------------------------------------------------------------------------------
//  structure & Enum
//-------------------------------------------------------------------------------------------------

typedef enum
{
    E_HAL_CAMCLK_SUPPORTED_LPLL_HS_CH_100TO187D5MHZ, //0
    E_HAL_CAMCLK_SUPPORTED_LPLL_HS_CH_50TO100MHZ,    //1
    E_HAL_CAMCLK_SUPPORTED_LPLL_HS_CH_25TO50MHZ,     //2
    E_HAL_CAMCLK_SUPPORTED_LPLL_HS_CH_12D5TO25MHZ,   //3
    E_HAL_CAMCLK_SUPPORTED_LPLL_MAX,                      //4
} HalCamClkLpllType_e;

//-------------------------------------------------------------------------------------------------
//  Prototype
//-------------------------------------------------------------------------------------------------
typedef struct
{
    u32  address;
    u16 value;
}HalCamClkLpllTbl_t;

#endif //_LPLL_TBL_H_
