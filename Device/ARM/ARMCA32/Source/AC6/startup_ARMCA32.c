/******************************************************************************
 * @file     startup_ARMCA32.c
 * @brief    CMSIS Device System Source File for ARM Cortex-A32 Device Series
 * @version  V1.00
 * @date     10 Nov 2017
 *
 * @note
 *
 ******************************************************************************/
/*
 * Copyright (c) 2009-2017 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <ARMCA32.h>

/*----------------------------------------------------------------------------
  Definitions
 *----------------------------------------------------------------------------*/
#define USR_MODE 0x10            // User mode
#define FIQ_MODE 0x11            // Fast Interrupt Request mode
#define IRQ_MODE 0x12            // Interrupt Request mode
#define SVC_MODE 0x13            // Supervisor mode
#define ABT_MODE 0x17            // Abort mode
#define UND_MODE 0x1B            // Undefined Instruction mode
#define SYS_MODE 0x1F            // System mode

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/

/** \brief Exception and Interrupt Handler Jumptable.
*/
void Vectors       (void) __attribute__ ((naked, section("RESET")));

/** \brief Reset Handler
*/
void Reset_Handler (void) __attribute__ ((naked));

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
void Undef_Handler       (void) __attribute__ ((weak, alias("Default_Handler")));
void SVC_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void PAbt_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void DAbt_Handler        (void) __attribute__ ((weak, alias("Default_Handler")));
void Hypervisor_Handler  (void) __attribute__ ((weak, alias("Default_Handler")));
void IRQ_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));
void FIQ_Handler         (void) __attribute__ ((weak, alias("Default_Handler")));

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector Table
 *----------------------------------------------------------------------------*/
void Vectors(void) {
  __ASM volatile(
  "LDR    PC, =Reset_Handler                        \n"
  "LDR    PC, =Undef_Handler                        \n"
  "LDR    PC, =SVC_Handler                          \n"
  "LDR    PC, =PAbt_Handler                         \n"
  "LDR    PC, =DAbt_Handler                         \n"
  "LDR    PC, =Hypervisor_Handler                   \n"
  "LDR    PC, =IRQ_Handler                          \n"
  "LDR    PC, =FIQ_Handler                          \n"
  );
}

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) {
  __ASM volatile(

  // Mask interrupts
  "CPSID   if                                      \n"

  // Put any cores other than 0 to sleep
  "MRC     p15, 0, R0, c0, c0, 5                   \n"  // Read MPIDR
  "ANDS    R0, R0, #3                              \n"
  "goToSleep:                                      \n"
  "WFINE                                           \n"
  "BNE     goToSleep                               \n"

  // Reset SCTLR Settings
  "MRC     p15, 0, R0, c1, c0, 0                   \n"  // Read CP15 System Control register
  "BIC     R0, R0, #(0x1 << 12)                    \n"  // Clear I bit 12 to disable I Cache
  "BIC     R0, R0, #(0x1 <<  2)                    \n"  // Clear C bit  2 to disable D Cache
  "BIC     R0, R0, #0x1                            \n"  // Clear M bit  0 to disable MMU
  "BIC     R0, R0, #(0x1 << 13)                    \n"  // Clear V bit 13 to disable hivecs
  "MCR     p15, 0, R0, c1, c0, 0                   \n"  // Write value back to CP15 System Control register
  "ISB                                             \n"

  // CPUECTLR.SMPEN bit must be set before the caches and MMU are enabled,
  // or any cache and TLB maintenance operations are performed, even for "AMP" CPUs.
  "MRRC    p15, 1, r0, r1, c15                     \n"  // Read CPUECTLR
  "ORR     r0, r0, #(1 << 6)                       \n"  // Set CPUECTLR.SMPEN bit
  "MCRR    p15, 1, r0, r1, c15                     \n"  // Write CPUECTLR

  // Set Vector Base Address Register (VBAR) to point to this application's vector table
  "LDR    R0, =Vectors                             \n"
  "MCR    p15, 0, R0, c12, c0, 0                   \n"

  // Setup Stack for each exceptional mode
  "CPS    #0x11                                    \n"
  "LDR    SP, =Image$$FIQ_STACK$$ZI$$Limit         \n"
  "CPS    #0x12                                    \n"
  "LDR    SP, =Image$$IRQ_STACK$$ZI$$Limit         \n"
  "CPS    #0x13                                    \n"
  "LDR    SP, =Image$$SVC_STACK$$ZI$$Limit         \n"
  "CPS    #0x17                                    \n"
  "LDR    SP, =Image$$ABT_STACK$$ZI$$Limit         \n"
  "CPS    #0x1B                                    \n"
  "LDR    SP, =Image$$UND_STACK$$ZI$$Limit         \n"
  "CPS    #0x1F                                    \n"
  "LDR    SP, =Image$$ARM_LIB_STACK$$ZI$$Limit     \n"

  // Call SystemInit
  "BL     SystemInit                               \n"

  // Unmask interrupts
  "CPSIE  if                                       \n"

  // Call __main
  "BL     __main                                   \n"
  );
}

/*----------------------------------------------------------------------------
  Default Handler for Exceptions / Interrupts
 *----------------------------------------------------------------------------*/
void Default_Handler(void) {
  while(1);
}
