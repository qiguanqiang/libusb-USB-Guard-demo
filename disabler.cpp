#include "disabler.h"


/* 从VID和PID判断设备是否被禁用（存储在文件中）
    返回值：DISABLED：被禁用了；ENABLED：未被禁用。 */
int disabler::is_device_disabled(int vid, int pid) {
    dev_dis_info tmp;

    ifstream file_in(DISABLE_LIST_FILE_PATH);
    if(!file_in.is_open()) {
        return -1;
    }

    while(!file_in.eof()) {
        file_in >> tmp.vid >> tmp.pid;
        if(tmp.vid == vid && tmp.pid == pid) {
            return DISABLED;
        }
    }
    file_in.close();
    return ENABLED;
}

/* 以追加方式添加设备禁用记录，此函数应在查询设备为ENABLED后才可以使用。
    返回值：成功则返回EXIT_SUCCESS */
int disabler::disable_record(int vid, int pid) {
    /*dev_dis_info tmp;
    vector<dev_dis_info> vec;

    ifstream file_in("disable_list");
    if(!file_in.is_open()) {
        return -1;
    }

    while(!file_in.eof()) {
        file_in >> tmp.vid >> tmp.pid >> tmp.disable;
        vec.push_back(tmp);
    }
    vec.popback();
    for(auto iter = vec.cbegin(); iter != vec.cend(); iter++) {

    }*/
    ofstream file_out(DISABLE_LIST_FILE_PATH, ios::out|ios::app);
    file_out << vid << " " << pid << endl;
    file_out.close();
    return EXIT_SUCCESS;
}

/* 以追加方式删除设备禁用记录，此函数应在查询设备为DISABLED后才可以使用。
    返回值：成功则返回EXIT_SUCCESS；失败则返回EXIT_FAILURE */
int disabler::enable_record(int vid, int pid) {
    dev_dis_info tmp;
    vector<dev_dis_info> vec;

    ifstream file_in(DISABLE_LIST_FILE_PATH);
    if(!file_in.is_open()) {
        return -1;
    }

    while(!file_in.eof()) {
        file_in >> tmp.vid >> tmp.pid;
        vec.push_back(tmp);
    }
    vec.pop_back();
    file_in.close();

    ofstream file_out(DISABLE_LIST_FILE_PATH, ios::out);
    if(vec.size() == 1) { //only one element
        tmp = vec.back();
        if(tmp.vid == vid && tmp.pid == pid) {
            file_out.close();
            return EXIT_SUCCESS;
        }else {
            return EXIT_FAILURE;
        }
    }else {
        for(auto iter = vec.begin(); iter != vec.end(); iter++) {
            //qDebug() << iter->vid << " " << iter->pid;
            if(iter->vid != vid || iter->pid != pid) {
                file_out << iter->vid << " " << iter->pid << endl;
            }

        }
        file_out.close();
        return EXIT_SUCCESS;
    }
}


