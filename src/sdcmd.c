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

#include "sd.h"

int sdOpenDevice(char* filename)
{
    int fd;

    fd = open(filename, 0);
    if(fd < 0) {
        perror("[-] Failed to open MMC block device");
        exit(1);
    }
    return fd;
}

void sdExitDeviceError(int fd, const char* error)
{
    printf("Error %d\n",errno);
    perror(error);
    close(fd);
    exit(-1);
}

int sdSendCommand(int cardfd, uint8_t cmd, uint32_t arg, uint8_t* data, uint16_t dataLen, uint8_t dataWr, uint32_t replyType, uint32_t* reply)
{
    int ioctlVal;
    struct mmc_ioc_cmd cmdStruct;
    memset(&cmdStruct, 0, sizeof(cmdStruct));

    if(cmd & 0x80) {
        cmdStruct.is_acmd=1;
        cmd &= 0x7F;
    }

    cmdStruct.write_flag = (dataWr>0);

    cmdStruct.opcode = cmd;
    cmdStruct.arg = arg;

    if(cmd == SD_READ_CSD || cmd == SD_WRITE_CSD) {
        //CSD Commands seem to have long delays on some cards.
        cmdStruct.postsleep_min_us=1e5;
        cmdStruct.postsleep_max_us=1e5;
        cmdStruct.cmd_timeout_ms=1e3;
        cmdStruct.data_timeout_ns=1e9;
    }

    cmdStruct.flags|=replyType;

    if(data != NULL) {
        cmdStruct.blksz=dataLen;
        cmdStruct.blocks=1;
        cmdStruct.flags |= MMC_COMMAND_ADTC;
        mmc_ioc_cmd_set_data(cmdStruct, data);
    }

    if( (ioctlVal = ioctl(cardfd, MMC_IOC_CMD, &cmdStruct) ) < 0) {
        sdExitDeviceError(cardfd, "[-] Failed to send command");
    }
    if(reply) memcpy(reply, cmdStruct.response, 16);

    return ioctlVal;
}

uint16_t sdFindRca(char* filename)
{
    unsigned int rca;
    unsigned int controller;
    DIR *dir, *dir2;
    char devname[16];
    char blockDevPath[512];
    struct dirent *entry;
    int i;
    for(i=strlen(filename)-1; (i>0) && filename[i-1]!='/'; i--);
    strncpy(devname,filename+i,sizeof(devname));
    if ((dir = opendir ("/sys/bus/mmc/devices/")) != NULL) {
        while ((entry = readdir (dir)) != NULL) {
            if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..")) continue;
            snprintf (blockDevPath, sizeof(blockDevPath), "/sys/bus/mmc/devices/%s/block/%s", entry->d_name,devname);
            if((dir2 = opendir (blockDevPath)) != NULL) {
                sscanf(entry->d_name,"mmc%u:%4x",&controller,&rca);
                closedir(dir);
                closedir(dir2);
                return rca;
            }
        }
        closedir (dir);
    } else {
        perror("[-] Failed to open /sys/bus/mmc/devices/. Could not determine RCA");
        exit(1);
    }
    printf("[-] RCA not found. Make sure /sys/ is mounted. Run this program only on the main volume, eg. /dev/mmcblk0\n");
    exit(1);
}


void sdActivate(int cardfd, uint16_t cardRca)
{
    sdSendCommand(cardfd, SD_SELECT, cardRca<<16, NULL, 0, 0, MMC_COMMAND_REPLY_R1B, NULL);
}

void sdStandbyAll(int cardfd)
{
    sdSendCommand(cardfd, SD_SELECT, 0, NULL, 0, 0 , MMC_COMMAND_REPLY_NONE, NULL);
}

void sdCSDSetCRC(uint8_t* csd)
{
    uint8_t crc=0;
    uint8_t i;
    for(i=0; i<15; i++) {
        crc=crc7AddWord(crc,csd[i],8);
    }
    csd[15] = (crc<<1)|1;
}

void sdReadCSD(int cardfd, uint16_t cardRca, uint8_t* csd)
{
    uint32_t reply[4];
    uint8_t i;
    sdSendCommand(cardfd, SD_READ_CSD, cardRca<<16, NULL, 0, 0, MMC_COMMAND_REPLY_R2, reply);
    for(i=0; i<16; i++) {
        csd[i]=reply[i/4]>>((3-(i%4))*8);
    }

}

void sdWriteCSD(int cardfd, uint8_t* csd)
{
    sdSendCommand(cardfd, SD_WRITE_CSD, 0, csd, 16, 1, MMC_COMMAND_REPLY_R1, NULL);
}

void sdGoIdle(int cardfd)
{
    sdSendCommand(cardfd, SD_GO_IDLE, 0, NULL, 0, 0, MMC_COMMAND_REPLY_NONE, NULL);
}
