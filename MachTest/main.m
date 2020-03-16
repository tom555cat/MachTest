
//
//  main.m
//  MachTest
//
//  Created by tongleiming on 2020/2/25.
//  Copyright © 2020 tongleiming. All rights reserved.
//
//#import <Foundation/Foundation.h>
#import <stdio.h>

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
        //getProcessorInfo();
        
        //
        processorSetInfo();
    }
    return 0;
}
