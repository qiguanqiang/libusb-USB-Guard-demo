#ifndef MAIN_FUNCTIONS_H
#define MAIN_FUNCTIONS_H

#include <libs.h>
#include <disabler.h>
#include "mainwindow.h"

void device_init(libusb_device **&devs);
int uninstall_device(libusb_device *dev);
int install_device(libusb_device *dev);
int get_device_list(libusb_device **&devs, libusb_context *&context);
void print_devices(libusb_device *dev);
int *get_vid_pid(libusb_device *dev);
libusb_device *get_device_by_vid_pid(libusb_device **devs, int vid, int pid);
libusb_device *get_device_by_vid_pid_2(libusb_device **devs, int vid, int pid, libusb_context *context);
void *listen_hotplug(void *args, libusb_context *context);
QString get_device_type(libusb_device* dev);

#endif // MAIN_FUNCTIONS_H
