#ifndef DISABLER_H
#define DISABLER_H
/* 这个类最初是打算作为后端逻辑部分实现，但是没有完成很好的设计，只做了一部分
    所以比较残缺，其他后端部分放在了‘main_functions’中，作为非成员函数提供
    给前端或者main调用。
    这个类包含几个功能，分别是检测设备是否被禁用、设置设备为启用和设置设备为
    禁用，都是通过读写文件实现。 */
#include "libs.h"
#include "vector"
#include "fstream"
#include "iostream"
#include "libusb.h"

using namespace std;
//#include "mainwindow.h"

/* 一些简单的宏定义 */

#define DISABLE_LIST_FILE_PATH "disable_list"

#define OPEN_FILE_ERROR -1

//In function is_device_disabled(int vid, int pid):
#define IN_LIST 0
#define NOT_IN_LIST 1

//In function device_in_list(int vid, int pid):
#define ENABLED 0
#define DISABLED 1
#define NOT_FOUND -2


/* 作为读写文件的结构体缓存读入的数据 */
struct dev_dis_info {
    int vid;
    int pid;
    //usb device's idVendor and idProduct
    //int disable;
    //flag of usb device disabled or not
    //1,if disabled; 0, if enabled;
};




class disabler {
private:

public:
    int is_device_disabled(int vid, int pid);
    int disable_record(int vid, int pid);
    int enable_record(int vid, int pid);
};


#endif // DISABLER_H
