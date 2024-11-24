/*
 * Copyright (c) 2022 
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys_init.h>
#include <cm4.h>
#include <sys_clock.h>
#include <sys_usart.h>
#include <serial_lin.h>
#include <sys_gpio.h>
#include <kstdio.h>
#include <debug.h>
#include <timer.h>
#include <UsartRingBuffer.h>
#include <system_config.h>
#include <mcu_info.h>
#include <sys_rtc.h>
#include <kflash.h>
#include <stdint.h>
#include<kcrc.h>
#ifndef DEBUG
#define DEBUG 1
#endif
extern UART_HandleTypeDef huart6;

void display_status(void);
int check_version(void);
void system_update(void);

char updated_os[100005];
int os_size;
//To store the latest version
char latest_version[50];
char response[50];
uint8_t isupdate = 0;

void __sys_init(void)
{
	__init_sys_clock(); //configure system clock 180 MHz
	__ISB();	
	__enable_fpu(); //enable FPU single precision floating point unit
	__ISB();
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	__SysTick_init(1000);	//enable systick for 1ms
	//SYS_RTC_init();
	SerialLin2_init(__CONSOLE,0);
	SerialLin6_init(&huart6,0);
	Ringbuf_init(__CONSOLE);
	Ringbuf_init(&huart6);
	ConfigTimer2ForSystem();
	__ISB();
	#ifdef DEBUG
	display_status();
    //Check if the version is updated
    if (check_version() == 1) {
        system_update();
        isupdate = 1;
    }
	#endif
}

void __sys_disable(void)
{
 
	// DISABLE ALL PERIPHERALS
	DisableUart(&huart2);
	DisableUart(&huart6);
 
	// DISABLE TIMER2
	DisableTimer2();
    
	// DISABLE ALL INTERRUPTS
	NVIC_DisableIRQ(USART2_IRQn);
	NVIC_DisableIRQ(USART6_IRQn);
	NVIC_DisableIRQ(TIM2_IRQn);
	NVIC_DisableIRQ(SysTick_IRQn);
	NVIC_DisableIRQ(FPU_IRQn);
 
	ms_delay(5000);
}

/*
* Do not remove it is for debug purpose
*/

void SYS_ROUTINE(void)
{
	__debugRamUsage();
}


void display_status(void)
{
    kprintf("###################### Bootloader Activated (29, 47, 57) ###########################\n");
}

//Compare two strings
int compare_strings(const char *str1, const char *str2) {
    int i = 0;

    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i]) {
            return (str1[i] - str2[i]);
        }
        i++;
    }

    return 0;
}

int check_version(void) {
    //Start address of sector 7 where the OS version is stored
    uint32_t start_address = 0x08060000U;
    //Get the current version of the OS
    char* current_version = get_os_version(start_address);
    //Send the current version to the server
    kprintf("CHECK_VERSION %s\n", current_version);
    // kprintf("CHECK_VERSION 1.0\n");

    int i = 0;
    char c = "";
    do {
        kscanf("%c", &c);
        response[i++] = c;
    } while (c != '\n');

    //Print the response from the server
    kprintf("%s", response);

    int ret = compare_strings(response, "UPDATE AVAILABLE");

    if (ret == 0) {
        //If update is available, return 1
        return 1;
    }

    return 0;
}

int char_array_to_int(const char *str, int n) {
    int result = 0;
    int i = 0;

    while (i < n) {
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
        }
        i++;
    }

    return result;
}

void system_update(void) {
    //Send the request to the server to get the updated OS
    kprintf("GET_UPDATE\n");

    //Track the byte position
    int i = 0;
    //Use to track the CRC data
    int s = 0;
    //Store the size of the file
    int file_size = 0;

    // read file size
    char c = "";
    char len[10];
    int j = 0;
    do {
        kscanf("%c", &c);
        if (c != '$') {
            len[j++] = c;
        }
    } while (c != '$');

    file_size = char_array_to_int(len, j);
    os_size = file_size;

    kprintf("ACK\n");

    //500 bytes of data and 4 bytes of CRC data
    const int CHUNK_SIZE = 504;

    while (file_size != 0) {
        c = "";
        for (int k = 0; k < CHUNK_SIZE; k++) {
            kscanf("%c", &c);
            updated_os[i++] = c;
        }
        int k = 0;
        //4 byte CRC data at the end of the chunk
        i = i-4;
        uint32_t crc_data = 0;
        //Get the CRC data and store it in crc_data that is 32 bit unsigned integer
        while(k < 4) {
            crc_data |= (updated_os[i + k] << (8 * (3-k)));
            k++;
        }
        //Initialize the CRC32
        CRC32_Init();
        //Process the data 4bytes at a time
        for(;s<i;s=s+4){
            k = 0;
            uint32_t crc_content = 0;
            while(k < 4) {
                //Making 32bit unsigned integer from 4 bytes
                crc_content |= (updated_os[s + k] << (8 * (3-k)));
                k++;
            }
            //Process the data
            CRC32_Process(crc_content);
        }
        //Finalize the CRC32 calculation
        if(CRC32_Finalize(crc_data) != 0){
            //If the CRC data is not matched, then send NACK and adjust the byte position
            i = i-500;
            s = i;
            // kprintf("Server crc: %x\n", crc_data);
            // kprintf("Bootloader crc: %x\n", CRC32_get());
            kprintf("NACK\n");
        }
        else{
            //If the CRC data is matched, then send ACK and adjust the file size
            if (file_size >= CHUNK_SIZE) {
                file_size -= CHUNK_SIZE;
            }
            else {
                file_size = 0;
            }
            // kprintf("Server crc: %x\n", crc_data);
            // kprintf("Bootloader crc: %x\n", CRC32_get());
            kprintf("ACK\n");
        }

    }

    //Download complete
    kprintf("Download complete.Total pakage size is %d Bytes\n", os_size);
}

char* get_updated_os(void) {
    //Return the updated OS
    return updated_os;
}

int get_size(void) {
    //Return the size of the updated OS
    return os_size;
}

//Return the latest version
char* get_latest_version(void) {
    int i = 0;
    int j = 0;

    while(response[i] != '\n' && response[i] != NULL) {
        if((response[i] >= 48 && response[i] <= 57) || response[i] == 46){
            latest_version[j++] = response[i];
        }
        i++;
    }
    return latest_version;
}

//Return the update status
int get_update_status(void) {
    return isupdate;
}
    