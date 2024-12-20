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
#include <kmain.h>
#include <stdint.h>
#include <sys_usart.h>
#include <kstdio.h>
#include <sys_rtc.h>
#include <kstring.h>
#include <kflash.h>
#ifndef DEBUG
#define DEBUG 1
#endif

#define FLASH_BASE_ADDRESS          (0x08000000U)
#define BOOTLOADER_SIZE             (0x00010000U) // 64 KB
#define OS_START_ADDRESS    (FLASH_BASE_ADDRESS + BOOTLOADER_SIZE) // 0X08010000


void jump_to_os(void) {
    //Declare a function pointer
    typedef void (*void_fn)(void);
    //Get the reset vector address
    uint32_t* reset_vector_entry = (uint32_t*)(OS_START_ADDRESS + 4U);
    //Get the reset vector
    uint32_t* reset_vector = (uint32_t*)(*reset_vector_entry);
    //Cast the reset vector to a function pointer
    void_fn jump_fn = (void_fn)reset_vector;

    //Jump to the OS
    jump_fn();
}




void kmain(void)
{   
    // Initialize the system
    __sys_init();
    // Get the start address of the version
    uint32_t start_address = 0x08060000U;
    ms_delay(1000);

    // Check if the OS is updated
    int isupdate = get_update_status();
    // If the OS is updated
    if(isupdate==1){
        // Get the updated OS
        char* updated_os = get_updated_os();
        ms_delay(1000);
        // Get the length of the updated OS
        int len = get_size()-1000;
        // Get the latest version
        char *version = get_latest_version();
        ms_delay(1000);
        erase_version_flash();
        ms_delay(1000);
        // Write the version to the flash in sector 7
        flash_write((uint8_t*) version,__strlen((uint8_t*) version), start_address);
        ms_delay(1000);
        kprintf("Updating to version: %s\n", version);
        ms_delay(500);
        kprintf("Cleaning old package\n");
        erase_os_flash();
        kprintf("Clean complete\n");
        ms_delay(1000);
        kprintf("Installing new package\n");
        // Write the updated OS to the flash in sector 4, 5, 6
        flash_write((uint8_t*) updated_os + 0x1000, len, OS_START_ADDRESS);
        ms_delay(1000);
        kprintf("Successfully Installed\n");
    }

    kprintf("Switching to os\n");
    ms_delay(500);
    // Jump to the OS
    jump_to_os();
    ms_delay(1000);

    __sys_disable();

    while (1) { }
}