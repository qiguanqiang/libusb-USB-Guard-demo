#include "disabler.h"
int disabler::device_in_list(int vid, int pid) {

}

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

int diabler::disable_record(int vid, int pid) {
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

int diabler::enable_record(int vid, int pid) {
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
    vec.popback();
    file_in.close();

    ofstream file_out(DISABLE_LIST_FILE_PATH, ios::out);
    if(vec.size() == 1) { //only one element
        if(vec.end().vid && vec.end().pid == pid) {
            file_out.close();
            return EXIT_SUCCESS;
        }else {
            return EXIT_FAILURE;
        }
    }else {
        for(auto iter = vec.cbegin(); iter != vec.cend(); iter++) {
            if((*iter).vid != vid || (*iter).pid != pid) {
                file_out << (*iter).vid << " " << (*iter).pid << endl;
            }
            file_out.close();
            return EXIT_SUCCESS;
        }
    }
}

