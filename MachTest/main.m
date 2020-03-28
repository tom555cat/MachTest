
//
//  main.m
//  MachTest
//
//  Created by tongleiming on 2020/2/25.
//  Copyright © 2020 tongleiming. All rights reserved.
//
//#import <Foundation/Foundation.h>
#import <stdio.h>
#include <mach/vm_region.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <mach/mach.h>

//int main(int argc, const char * argv[]) {
//    @autoreleasepool {
//        // insert code here...
//        NSLog(@"Hello, World!");
//
//        //HostInfo();
//        //getHostPrivPortAndReboot();
//
//        // 以root用户运行的结果为
//        //        Master processors Intel x86-64h Haswell in slot 0
//        //        Slave processors Intel x86-64h Haswell in slot 1
//        //        Slave processors Intel x86-64h Haswell in slot 2
//        //        Slave processors Intel x86-64h Haswell in slot 3
//        //        Slave processors Intel x86-64h Haswell in slot 4
//        //        Slave processors Intel x86-64h Haswell in slot 5
//        //        Slave processors Intel x86-64h Haswell in slot 6
//        //        Slave processors Intel x86-64h Haswell in slot 7
//        //getProcessorInfo();
//
//        //
//        processorSetInfo();
//    }
//    return 0;
//}


/// 与show_regions()函数相关的测试代码
int main(int argc, const char * argv[]) {
    struct vm_region_basic_info vmr;
    kern_return_t rc;
    mach_port_t task;
    mach_vm_size_t size = 8;
    vm_region_info_t info = (vm_region_info_t)malloc(10000);
    mach_msg_type_number_t info_count;
    mach_port_t object_name;
    mach_vm_address_t addr = 1;
    int pid;
    if (!argv[1]) {
        printf("Usage: %s <PID>\n");
        exit(1);
    }
    pid = atoi(argv[1]);
    
    // 通过task_for_pid()获得任务端口
    rc = task_for_pid(mach_task_self(), pid, &task);
    
    if (rc) {
        fprintf(stderr, "task_for_pid() failed with error %d - %s (Am I entitled?)\n", rc, mach_error_string(rc));
        exit(1);
    }
    printf("Task: %d\n", task);
    //show_regions(task, addr);
    printf("Done\n");
    
    return 0;
}

