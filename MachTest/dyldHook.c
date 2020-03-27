//
//  dyldHook.c
//  MachTest
//
//  Created by tongleiming on 2020/3/16.
//  Copyright © 2020 Hello World Corporation. All rights reserved.
//

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc/malloc.h>

typedef struct interpose_s
{
    void *new_func;
    void *orig_func;
} interpose_t;

// 我们的原型，要求有原型的原因是要将其放在下面的interposing_functions中
void *my_malloc(int size);   // 对应真实的malloc（）
void my_free(void *);        // 对应真实的free()

static const interpose_t interposing_functions[] __attribute__ ((section("__DATA, __interpose"))) = {
    {(void *)my_free, (void *)free},
    {(void *)my_malloc, (void *)malloc},
};

void *my_malloc(int size) {
    // 在我们的函数中，要访问真正的malloc()j函数，因为不想自己管理整个堆
    // 所以就调用了原来的malloc()
    void *returned = malloc(size);
    
    // 调用malloc_printf()的原因是真实的printf()内部会调用malloc()，
    // 这会调用回自己，产生无限递归调用
    malloc_printf("+ %p %d\n", returned, size);
    return (returned);
}

void my_free(void *freed) {
    // 释放内存---打印出地址，然后调用真正的free()
    malloc_printf("- %p\n", freed);
    free(freed);
}
