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

int displayWpState(uint8_t* csd)
{
    int retVal;
    printf("[+] Write protection state: ");
    if(csd[14]&0x20) {
        printf("Permanent");
        retVal=-1;
    } else if(csd[14]&0x10) {
        printf("Temporary");
        retVal=-2;
    } else {
        printf("Off");
        retVal=-3;
    }
    printf(".\n");
    return retVal;
}

void printHelp()
{
    printf("This program requires two arguments. The first argument is the device to use. The second argument one of the following actions:\n");
    printf("  status:   show current write protection setting\n");
    printf("  unlock:   disable write protection\n");
    printf("  lock:     enable temporary write protection\n");
    printf("  permlock: enable permanent write protection\n");
    printf("  reset:    send go_idle_state command to all cards\n\n");
    printf("Example: ./sdtool /dev/mmcblk0 lock\n");
}

void permlockWarning(uint8_t* csd)
{
    if(csd[14]&0x20) {
        printf("[?] Card is permanently locked. I will try to clear the flag, but it will likely fail.\n");
    }
}

int main(int argc, char *argv[])
{
    int cardfd;
    uint16_t cardRca;
    uint8_t csd[16];
    uint8_t action;
    uint8_t oldCSD14;
    uint8_t i;

    if(argc!=3) {
        printHelp();
        exit(0);
    }
    cardfd = sdOpenDevice(argv[1]);
    if(!strcmp(argv[2],"status")) {
        action=0;
    } else if(!strcmp(argv[2],"unlock")) {
        action=1;
    } else if(!strcmp(argv[2],"lock")) {
        action=2;
    } else if(!strcmp(argv[2],"permlock")) {
        action=3;
    } else if(!strcmp(argv[2],"reset")) {
        printf("[+] Done.\n");
        sdGoIdle(cardfd);
        exit(0);
    } else {
        printf("Invalid command: %s\n\n\n",argv[2]);
        printHelp();
        exit(0);
    }
    cardRca=sdFindRca(argv[1]);
    printf("[+] Found RCA for %s: %04X.\n",argv[1],cardRca);

    sdStandbyAll(cardfd);
    sdReadCSD(cardfd, cardRca, csd);
    sdActivate(cardfd, cardRca);
    oldCSD14=csd[14];
    switch (action) {
        case 0:
            sdCSDSetCRC(csd);
            printf("[+] Card CSD: ");
            for(i=0; i<16; i++) {
                printf("%02X",csd[i]);
            }
            printf(".\n");
            exit(displayWpState(csd));
            break;
        case 1:
            permlockWarning(csd);
            csd[14] &=~0x30;
            break;
        case 2:
            permlockWarning(csd);
            csd[14] &=~0x30;
            csd[14] |= 0x10;
            break;
        case 3:
            csd[14] &=~0x30;
            csd[14] |= 0x20;
            break;
    }
    if(csd[14]!=oldCSD14) {
        printf("[+] Writing CSD.\n");
        sdCSDSetCRC(csd);
        sdWriteCSD(cardfd, csd);
    } else {
        printf("[+] CSD unchanged.\n");
    }
    return displayWpState(csd);
}
