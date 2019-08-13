#include "disabler.h"
int disabler::device_in_list(int vid, int pid) {

}

int disabler::is_device_disabled(int vid, int pid) {
    dev_dis_info tmp;

    ifstream file_in("disable_list");
    if(!file_in.is_open()) {
        return -1;
    }

   /* vector<dev_dis_info> vec;
    while(!file_in.eof()) {
        file_in >> tmp.vid >> tmp.pid >> tmp.disable;
        vec.push_back(tmp);
    }
    vec.pop_back();*/
    while(!file_in.eof()) {
        file_in >> tmp.vid >> tmp.pid;
        if(tmp.vid == vid && tmp.pid == pid) {
            return DISABLED;
        }
    }
    file_in.close();
    return ENABLED;
}

