//
//  vmmapExample.c
//  MachTest
//
//  Created by tongleiming on 2020/3/27.
//  Copyright © 2020 Hello World Corporation. All rights reserved.
//

#include <mach/mach_types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/mach.h>
#include "vmmapExample.h"


void show_regions(task_t task, mach_vm_address_t address) {
    kern_return_t kr;
    vm_region_basic_info_data_t info, prev_info;
    mach_vm_address_t prev_address;
    mach_vm_size_t size, prev_size;
    
    mach_port_t object_name;
    mach_msg_type_number_t count;
    
    int nsubregions = 0;
    int num_printed = 0;
    int done = 0;
    
    count = VM_REGION_BASIC_INFO_COUNT_64;
    
    kr = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &count, &object_name);
    if (kr != KERN_SUCCESS) {
        printf("Error %d - %s", kr, mach_error_string(kr));
        return;
    }
    memcpy(&prev_info, &info, sizeof(vm_region_basic_info_data_t));
    prev_address = address;
    prev_size = size;
    nsubregions = 1;
    
    while (!done) {
        int print = 0;
        address = prev_address + prev_size;
        
        // 检查地址空间是否发生绕回
        if (address == 0) {
            print = done = 1;
        }
        
        if (!done) {
            // 在iOS中也要使用VM_REGION_BASIC_INFO_COUNT_64
            count = VM_REGION_BASIC_INFO_COUNT_64;
            
            kr = mach_vm_region(task, &address, &size, VM_REGION_BASIC_INFO, (vm_region_info_t)&info, &count, &object_name);
            
            if (kr != KERN_SUCCESS) {
                fprintf(stderr, "mach_vm_region failed for address %p - error %d\n", address, kr);
                size = 0;
                print = done = 1;
            }
        }
        
        if (address != prev_address + prev_size) {
            print = 1;
        }
                    
        // 如果区域设置中有任何变化，则打印出来
        if ((info.protection != prev_info.protection)
            || (info.max_protection != prev_info.max_protection)
            || (info.inheritance != prev_info.inheritance)
            || (info.shared != prev_info.reserved)
            || (info.reserved != prev_info.reserved)) {
            print = 1;
        }
        
        if (print) {
            int print_size;
            char *print_size_unit;
            if (num_printed == 0) {
                printf("Region");
            } else {
                printf("...");
            }
            
            print_size = prev_size;
            if (print_size > 1024) {
                print_size /= 1024;
                print_size_unit = "K";
            }
            if (print_size > 1024) {
                print_size /= 1024;
                print_size_unit = "M";
            }
            if (print_size > 1024) {
                print_size /= 1024;
                print_size_unit = "G";
            }
            
//            protection_bits_to_rwx(prev_info.protection),
//            protection_bits_to_rwx(prev_info.max_protection),
//            unparse_inheritance(prev_info.inheritance),
//            ,
//            behavior_to_xxx(prev_info.behavior)
            
            printf(" %p-%p [%d%s](%s, %s)",
                   (prev_address),
                   (prev_address + prev_size),
                   print_size,
                   print_size_unit,
                   
                   prev_info.shared ? "shared" : "private",
                   prev_info.reserved ? "reserved" : "not-reserved");
            
            if (nsubregions > 1) {
                printf(" (%d sub-regions)", nsubregions);
            }
            
            printf("\n");
            
            prev_address = address;
            prev_size = size;
            memcpy(&prev_info, &info, sizeof(vm_region_basic_info_data_t));
            nsubregions = 1;
            
            num_printed++;
        } else {
            prev_size += size;
            nsubregions++;
        }
        
        if (done) {
            break;
        }
    }
}

