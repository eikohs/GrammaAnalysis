# pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*常量、宏函数定义*/
#define RECUR_ANALY 0
#define LL_ANALY 1
#define LR_ANALY 2

#define Debug_Error(Msg)\
    printf("Error! %s\n", Msg);

/*全局变量*/
extern int ANALYSIS_MODE; // 指示执行的语法分析类型。0：递归；1：自顶向下；2：自底向上