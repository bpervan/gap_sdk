/*
 * Copyright (c) 2020, GreenWaves Technologies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of GreenWaves Technologies, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include "pmsis.h"
#include "chips/gap9/drivers/udma/udma_core.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RX_CHANNEL 0
#define TX_CHANNEL 1
#define CMD_CHANNEL 2

/*******************************************************************************
 * Driver data
 ******************************************************************************/

char *udma_chan_str[3] = {
    [RX_CHANNEL] = "RX_CHANNEL",
    [TX_CHANNEL] = "TX_CHANNEL",
    [CMD_CHANNEL] = "CMD_CHANNEL"};

PI_FC_TINY uint32_t __pi_udma_chan_lin[PI_NB_UDMA_CHAN_LIN_REGS];
PI_FC_TINY uint32_t __pi_udma_chan_2d = 0;
PI_FC_TINY uint32_t __pi_udma_chan_fifo = 0;

/*******************************************************************************
 * API implementation
 ******************************************************************************/
