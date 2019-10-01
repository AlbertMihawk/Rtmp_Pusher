//
// Created by lixiaoxu on 2019-08-15.
//

#ifndef HAWKEYEPLAYER_MARCO_H
#define HAWKEYEPLAYER_MARCO_H

#include <android/log.h>

//定义Java的log
//int __android_log_print(int prio, const char* tag, const char* fmt, ...)
//ANDROID_LOG_INFO
//ANDROID_LOG_ERROR
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,"NEPUSHER",##__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,"NEPUSHER",##__VA_ARGS__)

//标记线程模式
#define THREAD_MAIN 1
#define THREAD_CHILD 2

//定义释放指针的宏函数
#define DELETE(object) if(object){delete object;object = 0;}


//错误代码
#define ERROR_CODE_OK 0


#endif //HAWKEYEPLAYER_MARCO_H


