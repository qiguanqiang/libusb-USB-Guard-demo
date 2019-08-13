#include "libs.h"
#include "vector"
#include "fstream"

#define OPEN_FILE_ERROR -1

//In function is_device_disabled(int vid, int pid):
#define IN_LIST 0
#define NOT_IN_LIST 1

//In function device_in_list(int vid, int pid):
#define ENABLED 0
#define DISABLED 1
#define NOT_FOUND -2

struct dev_dis_info {
    int vid;
    int pid;
    //usb device's idVendor and idProduct
    //int disable;
    //flag of usb device disabled or not
    //1,if disabled; 0, if enabled;
};

#ifndef DISABLER_H
#define DISABLER_H

#endif // DISABLER_H

class disabler {
private:

public:
    int is_device_disabled(int vid, int pid);
    int device_in_list(int vid, int pid);
};

