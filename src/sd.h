/*
 * Copyright (c) 2014-2017, Bertold Van den Bergh (vandenbergh@bertold.org)
 * All rights reserved.
 * This work has been developed to support research funded by
 * "Fund for Scientific Research, Flanders" (F.W.O.-Vlaanderen).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the author nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR DISTRIBUTOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <linux/types.h>
#include <sys/ioctl.h>
#include <linux/mmc/ioctl.h>

#include "sdlinux.h"

#ifndef _SD_H
#define _SD_H

#define SD_GO_IDLE   (0)
#define SD_SELECT    (7)
#define SD_READ_CSD  (9)
#define SD_WRITE_CSD (27)

//crc.c
extern uint8_t crc7AddWord(uint8_t crc, uint8_t word, uint8_t len);

//sdcmd.c
extern int sdOpenDevice(char* filename);
extern void sdExitDeviceError(int fd, const char* error);
extern int sdSendCommand(int cardfd, uint8_t cmd, uint32_t arg, uint8_t* data, uint16_t dataLen, uint8_t dataWr, uint32_t replyType, uint32_t* reply);
extern uint16_t sdFindRca(char* filename);
extern void sdActivate(int cardfd, uint16_t cardRca);
extern void sdStandbyAll(int cardfd);
extern void sdCSDSetCRC(uint8_t* csd);
extern void sdReadCSD(int cardfd, uint16_t cardRca, uint8_t* csd);
extern void sdWriteCSD(int cardfd, uint8_t* csd);
extern void sdGoIdle(int cardfs);

//main.c
extern int displayWpState(uint8_t* csd);
extern void printHelp();
extern void permlockWarning(uint8_t* csd);

#endif
