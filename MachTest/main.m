
//
//  main.m
//  MachTest
//
//  Created by tongleiming on 2020/2/25.
//  Copyright © 2020 tongleiming. All rights reserved.
//
//#import <Foundation/Foundation.h>
#import <stdio.h>
#import <mach/mach.h>
#import <mach/processor.h>
#import <mach-o/arch.h>

/// 类似于HostInfo的工具
int HostInfo() {
    // 获取主机端口，在host_self()内部调用的是mach_host_self()
    mach_port_t self = host_self();
    kern_return_t rc;
    char buf[1024];
    struct host_basic_info *hi;
    unsigned int len = 1024;
    
    // 通过对host_self()得到的结果使用host_info就可以获得主机信息
    rc = host_info(self, HOST_BASIC_INFO, (host_info_t)buf, &len);
    if (rc != 0) {
        fprintf(stderr, "Nope\n");
        return (1);
    }
    
    hi = (host_basic_info_t)buf;
    
    printf("CPU:\t\t %d/%d\n", hi->avail_cpus, hi->max_cpus);
    printf("Physical CPUs:\t %d/%d\n", hi->physical_cpu, hi->physical_cpu_max);
    printf("Logical CPUs:\t %d/%d\n", hi->logical_cpu, hi->logical_cpu_max);
    printf("CPU type:\t %d/%d, Threadtype: %d\n", hi->cpu_type, hi->cpu_subtype, hi->cpu_threadtype);
    
    printf("Memory size:\t %d/%llu\n", hi->memory_size, hi->max_mem);
    
    return 0;
}
/// 获取主机特权端口然后重启
void getHostPrivPortAndReboot() {
    // 获取主机端口
    mach_port_t h = mach_host_self();
    mach_port_t hp;
    kern_return_t rc;
    
    // 获取主机特权端口，只有root用户执行时才有用
    rc = host_get_host_priv_port(h, &hp);
    
    if (rc == KERN_SUCCESS) {
        host_reboot(hp, 0);
    }
    
    // 如果是root用户，则不会执行到这里
    printf("sorry!\n");
}
/// 打印处理器对象信息
int getProcessorInfo() {
    kern_return_t kr;
    // host_name_port_t就是mach_port_t
    host_name_port_t host = mach_host_self();
    // host_priv_t还是mach_port_t
    host_priv_t host_priv;
    processor_port_array_t processors;
    natural_t count, infoCount;
    processor_basic_info_data_t basicInfo;
    int p;
    
    // 首先获得主机特权端口
    kr = host_get_host_priv_port(host, &host_priv);
    
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "host_get_host_priv_port %d (you should be root)", kr);
        exit(1);
    }
    
    // 尝试获取处理器数组
    kr = host_processors(host_priv, &processors, &count);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "host_processors %d", kr);
        exit(1);
    }
    
    // 获取到了处理器数组，遍历这个数组
    for (p = 0; p < count; p++) {
        infoCount = PROCESSOR_BASIC_INFO_COUNT;
        
        
        kr = processor_info(processors[p],                  // processor_t数据
                            PROCESSOR_BASIC_INFO,           // 请求信息
                            &host,                          // 主机
                            (processor_info_t)&basicInfo,   // 返回信息在这里
                            &infoCount);                    // Sizeof(basicInfo)
        
        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "?!\n");
            exit(3);
        }
        
        // 输出到屏幕
        printf("%s processors %s in slot %d\n", (basicInfo.is_master ? "Master" : "Slave"), NXGetArchInfoFromCpuType(basicInfo.cpu_type, basicInfo.cpu_subtype)->description, basicInfo.slot_num);
    }
    
    return 0;
}
int main(int argc, const char * argv[]) {
    @autoreleasepool {
        // insert code here...
        NSLog(@"Hello, World!");
        
        //HostInfo();
        //getHostPrivPortAndReboot();
        
        // 以root用户运行的结果为
        //        Master processors Intel x86-64h Haswell in slot 0
        //        Slave processors Intel x86-64h Haswell in slot 1
        //        Slave processors Intel x86-64h Haswell in slot 2
        //        Slave processors Intel x86-64h Haswell in slot 3
        //        Slave processors Intel x86-64h Haswell in slot 4
        //        Slave processors Intel x86-64h Haswell in slot 5
        //        Slave processors Intel x86-64h Haswell in slot 6
        //        Slave processors Intel x86-64h Haswell in slot 7
        getProcessorInfo();
    }
    return 0;
}
