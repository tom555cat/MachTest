//
//  HostTaskThreadInfo.h
//  MachTest
//
//  Created by tongleiming on 2020/2/28.
//  Copyright © 2020 Hello World Corporation. All rights reserved.
//

#ifndef HostTaskThreadInfo_h
#define HostTaskThreadInfo_h

#include <stdio.h>
#import <mach/mach.h>
#import <mach/processor.h>
#import <mach-o/arch.h>

/// 类似于HostInfo的工具
int HostInfo(void);

/// 获取主机特权端口然后重启
void getHostPrivPortAndReboot(void);

/// 打印处理器对象信息
int getProcessorInfo(void);

/// 打印处理器集中的任务信息
int processorSetInfo(void);

/// 输出Task详细信息
/// @param Task task_t
void doTaskInfo(task_t Task);

/// 打印任务中每个线程的详细信息
/// @param Task 任务
int doTaskThreadInfo(task_t Task);

#endif /* HostTaskThreadInfo_h */
