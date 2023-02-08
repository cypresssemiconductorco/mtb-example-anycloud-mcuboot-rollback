/***************************************************************************//**
* File Name:   cy_serial_flash_prog.c
*
* Description: Provides variables necessary to inform programming tools how to program the
*              attached serial flash memory. The variables used here must be placed at
*              specific locations in order to be detected and used by programming tools
*              to know that there is an attached memory and its characteristics. Uses the
*              configuration provided as part of BSP.
*
*******************************************************************************
* Copyright 2021-2023, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/**
* \addtogroup group_board_libs
* \{
* In addition to the APIs for reading and writting to memory at runtime, this library also 
* provides support for informing programming tools about the external memory so it can be
* be written at the same time as internal flash. This support can be enabled by defining 
* CY_ENABLE_EXMEM_PROGRAM while building the application. With this define in place, code
* will be generated in the .cy_sflash_user_data & .cy_toc_part2 sections. These locations 
* can be read by programming tools (eg: Cypress Programmer, OpenOCD, pyOCD) to know that 
* there is a memory device attached and how to program it.
* \} group_board_libs
*/

#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(CY_ENABLE_EXMEM_PROGRAM)

#include "cycfg_qspi_memslot.h"

typedef struct
{
    const cy_stc_smif_block_config_t * smifCfg; /* Pointer to SMIF top-level configuration */
    const uint32_t null_t; /* NULL termination */
} stc_smif_ipblocks_arr_t;

/*
 * This data can be placed anywhere in the internal memory, but it must be at a location that
 * can be determined and used for the calculation of the CRC16 checksum in the cyToc below. There
 * are multiple ways this can be accomplished including:
 * 1) Placing it in a dedicated memory block with a known address. (as done here)
 * 2) Placing it at an absolute location via a the linker script
 * 3) Using 'cymcuelftool -S' to recompute the checksum and patch the elf file after linking
 */
CY_SECTION(".cy_sflash_user_data") __attribute__( (used) )
const stc_smif_ipblocks_arr_t smifIpBlocksArr = {&smifBlockConfig, 0x00000000};

/*
 * This data is used to populate the table of contents part 2. When present, it is used by the boot
 * process and programming tools to determine key characteristics about the memory usage including
 * where the boot process should start the application from and what external memories are connected
 * (if any). This must consume a full row of flash memory row. The last entry is a checksum of the
 * other values in the ToC which must be updated if any other value changes. This can be done manually
 * or by running 'cymcuelftool -S' to recompute the checksum.
 */
CY_SECTION(".cy_toc_part2") __attribute__( (used) )
const uint32_t cyToc[128] =
{
    0x200-4,                /* Offset=0x0000: Object Size, bytes */
    0x01211220,             /* Offset=0x0004: Magic Number (TOC Part 2, ID) */
    0,                      /* Offset=0x0008: Key Storage Address */
    (int)&smifIpBlocksArr,  /* Offset=0x000C: This points to a null terminated array of SMIF structures. */
    0x10000000u,            /* Offset=0x0010: App image start address */
                            /* Offset=0x0014-0x01F7: Reserved */
    [126] =  0x000002C2,    /* Offset=0x01F8: Bits[ 1: 0] CLOCK_CONFIG (0=8MHz, 1=25MHz, 2=50MHz, 3=100MHz)
                                              Bits[ 4: 2] LISTEN_WINDOW (0=20ms, 1=10ms, 2=1ms, 3=0ms, 4=100ms)
                                              Bits[ 6: 5] SWJ_PINS_CTL (0/1/3=Disable SWJ, 2=Enable SWJ)
                                              Bits[ 8: 7] APP_AUTHENTICATION (0/2/3=Enable, 1=Disable)
                                              Bits[10: 9] FB_BOOTLOADER_CTL: UNUSED */
    [127] =  0x3BB30000     /* Offset=0x01FC: CRC16-CCITT (the upper 2 bytes contain the CRC and the lower 2 bytes are 0) */
};

#endif /* defined(CY_ENABLE_EXMEM_PROGRAM) */

#if defined(__cplusplus)
}
#endif
