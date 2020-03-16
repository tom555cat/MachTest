//
//  HostTaskThreadInfo.c
//  MachTest
//
//  Created by tongleiming on 2020/2/28.
//  Copyright © 2020 Hello World Corporation. All rights reserved.
//

#include "HostTaskThreadInfo.h"

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

/// 打印处理器集中的任务信息
int processorSetInfo() {
    host_t myhost = mach_host_self();
    mach_port_t psDefault;
    mach_port_t psDefault_control;
    // tasks中保存的是mach_port_t
    task_array_t tasks;
    mach_msg_type_number_t numTasks;
    int t;
    
    kern_return_t kr;
    
    // 获得默认的处理器集
    kr = processor_set_default(myhost, &psDefault);
    
    // 请求访问控制端口
    kr = host_processor_set_priv(myhost, psDefault, &psDefault_control);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "host_processor_set_priv - %d", kr);
        exit(1);
    }
    
    // 获得任务，在iOS上无法获得PID 0（kernel_task）
    kr = processor_set_tasks(psDefault_control, &tasks, &numTasks);
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "processor_set_tasks - %d \n", kr);
        exit(2);
    }
    
    // 遍历所有任务
    // ** 这段代码通过"pid_for_task"将任务端口映射到对应BSD进程ID
    for (t = 0; t < numTasks; t++) {
        int pid;
        // 从这里可以看出task对应的mach_port_t和PID是不相等的
        pid_for_task(tasks[t], &pid);
        printf("Task: %d pid: %d\n", tasks[t], pid);
        
        doTaskInfo(tasks[t]);
        doTaskThreadInfo(tasks[t]);
    }
    
    return 0;
}

/// 输出Task详细信息
/// @param Task task_t
void doTaskInfo(task_t Task) {
    mach_msg_type_number_t infoSize;
    
    char infoBuf[TASK_INFO_MAX];
    struct task_basic_info_64 *tbi;
    struct task_events_info *tei;
    
#if LION
    struct task_kernelmemory_info_t *tkmi;
    struct task_extmod_info *texi;
    struct vm_extmod_statistics *vec;
#endif
    
    kern_return_t kr;
    infoSize = TASK_INFO_MAX;
    kr = task_info(Task,
                   TASK_BASIC_INFO_64,
                   (task_info_t)infoBuf,
                   &infoSize);
    tbi = (struct task_basic_info_64 *)infoBuf;
    
    printf("\tSuspend Count: %d\n", tbi->suspend_count);
    printf("\tMemory: %lluM virtual, %lluK resident\n", tbi->virtual_size/(1024 * 1024), tbi->resident_size/1024);
    printf("\tSystem/User Time: %ld/%ld\n", tbi->system_time, tbi->user_time);
    
    infoSize = TASK_INFO_MAX;  // 需要重置。
    kr = task_info(Task,
                   TASK_EVENTS_INFO,
                   (task_info_t)infoBuf,
                   &infoSize);
    
    tei = (struct task_events_info *)infoBuf;
    printf("Faults: %d, Page-Ins: %d, COW: %d\n", tei->faults, tei->pageins, tei->cow_faults);
    printf("Messages : %d sent, %d received\n", tei->messages_sent, tei->messages_received);
    printf("Syscalls: %d Mach, %d UNIX\n", tei->syscalls_mach, tei->syscalls_unix);
    
#if LION
    infoSize = TASK_INFO_MAX;
    kr = task_info(Task,
                   TASK_KERNELMEMORY_INFO,
                   (task_info_t)infoBuf,
                   &infoSize);
    tkmi = (struct task_kernelmemory_info *)infoBuf;
    printf("Kernel memory: Private: %dK allocated %dK freed, Shared: %dK allocated, %dK freed\n",
           tkmi->total_palloc/1024, tkmi->total_pfree/1024,
           tkmi->total_salloc/1024, tkmi->total_sfree/1024);
    
    infoSize = TASK_INFO_MAX;
    kr = task_info(Task,
                   TASK_EXTMOD_INFO,
                   (task_info_t)infoBuf,
                   &infoSize);
    if (kr == KERN_SUCCESS) {
        printf("--OK\n");
    }
    texi = (struct vm_extmod_statistics *)infoBuf;
    ves = &(texi->extmod_statistics);
    
    if (ves->task_for_pid_count) {
        printf("Task has been looked up %ld times\n", ves->task_for_pid_count);
    }
    if (ves->task_for_pid_caller_count) {
        printf("Task has looked up others %ld times\n", ves->task_for_pid_caller_count);
    }
    if(ves->thread_creation_count || ves->thread_set_state_count) {
        printf("Task has been tampered with\n");
    }
    if (ves->thread_creation_caller_cout || ves->thread_set_state_caller_count) {
        printf("Task has tampered with others\n");
    }
#endif
}

/// 打印任务中每个线程的详细信息
/// @param Task 任务
int doTaskThreadInfo(task_t Task) {
    thread_act_array_t threads;
    mach_msg_type_number_t threadNum;
    char infoBuf[THREAD_INFO_MAX];
    mach_msg_type_number_t infoSize;
    
    struct thread_basic_info *tbi;
    
    int p;
    
    kern_return_t kr;
    
    kr = task_threads(Task, &threads, &threadNum);
    
    if (kr != KERN_SUCCESS) {
        fprintf(stderr, "task_threads failed!");
        exit(1);
    }
    
    for (p = 0; p < threadNum; p++) {
        infoSize = THREAD_INFO_MAX;
        kr = thread_info(threads[p], THREAD_BASIC_INFO, (thread_info_t)infoBuf, &infoSize);
        if (kr != KERN_SUCCESS) {
            fprintf(stderr, "thread_info failed!");
            exit(2);
        }
        tbi = (struct thread_basic_info *)infoBuf;
//        struct thread_basic_info {
//            time_value_t    user_time;      /* user run time */
//            time_value_t    system_time;    /* system run time */
//            integer_t       cpu_usage;      /* scaled cpu usage percentage */
//            policy_t        policy;         /* scheduling policy in effect */
//            integer_t       run_state;      /* run state (see below) */
//            integer_t       flags;          /* various flags (see below) */
//            integer_t       suspend_count;  /* suspend count for thread */
//            integer_t       sleep_time;     /* number of seconds that thread
//                                             *  has been sleeping */
//        };
        printf("\tThread %d info:\n", threads[p]);
        printf("\t\tuser_time/system_time: %d/%d\n",tbi->user_time, tbi->system_time);
        printf("\t\tcpu_usage: %d\n",tbi->cpu_usage);
        printf("\t\trun_state: %d\n",tbi->run_state);
        printf("\t\tsuspend_count: %d\n",tbi->suspend_count);
    }
    
    return 0;
}
