/*@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  MAIN FUNCTIONS  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@*/
#include <main_functions.h>
#include <iostream>
using namespace std;

void print_devices(libusb_device *dev)
{
    libusb_device_descriptor dev_desc;
    libusb_config_descriptor *config_desc;
    //const libusb_interface *interface;
    //const libusb_interface_descriptor *interface_desc;
    //const libusb_endpoint_descriptor *endpoint_desc;
    //libusb_device_handle *handle = NULL;
    int ret;

    ret = libusb_get_device_descriptor(dev, &dev_desc);
    if (ret < 0) {
        qDebug() << "error in detting device descriptor" << libusb_error_name(ret) << endl;
        return;
    }

    qDebug() << "Number of POS configs is " << dev_desc.bNumConfigurations << endl;
    qDebug() << "Device Vendor id): "       << dev_desc.idVendor << endl;
    qDebug() << "Product ID: "              << dev_desc.idProduct << endl;
    qDebug() << "Class ID: "                << dev_desc.bDeviceClass << endl;

    ret = libusb_get_config_descriptor(dev, 0, &config_desc);
    if (ret == LIBUSB_ERROR_NOT_FOUND) {
        /*0 on success
        LIBUSB_ERROR_NOT_FOUND if the configuration does not exist
        another LIBUSB_ERROR code on error*/
        qDebug() << "The configuration does not exist" << endl;
    }else if(ret < 0){
        qDebug() << "Error Code in libusb_get_config_descriptor: " << libusb_error_name(ret);
    }else{
        qDebug() << "Interface: " << config_desc->bNumInterfaces << endl;
    }
    qDebug() << "*********************************************************" << endl;
}

int *get_vid_pid(libusb_device *dev) {
    libusb_device_descriptor dev_desc;
    int ret;
    int vid_pid[2];

    ret = libusb_get_device_descriptor(dev, &dev_desc);
    if (ret < 0) {
        qDebug() << "error in detting device descriptor" << libusb_error_name(ret) << endl;
        vid_pid[0] = EXIT_FAILURE;
        vid_pid[1] = EXIT_FAILURE;
        return vid_pid;
    }else{
        vid_pid[0] = dev_desc.idVendor;
        vid_pid[1] = dev_desc.idProduct;
        return vid_pid;
    }
}

int uninstall_device(libusb_device *dev) {
    int *vid_pid;
    int ret;

    vid_pid = get_vid_pid(dev);
    libusb_device_handle *handle = NULL;
    libusb_open(dev, &handle);
    ret = libusb_detach_kernel_driver(handle, 0);
    if (ret < 0) {
        qDebug() << "Error in detach kernel driver to device " << dev << ": " << libusb_error_name(ret) <<endl;
        libusb_close(handle);
        return ret;
    }else{
        libusb_close(handle);
        return EXIT_SUCCESS;
    }
}

int install_device(libusb_device *dev) {
    int *vid_pid;
    int ret;

    vid_pid = get_vid_pid(dev);
    libusb_device_handle *handle = NULL;
    libusb_open(dev, &handle);
    ret = libusb_attach_kernel_driver(handle, 0);
    if (ret < 0) {
        qDebug() << "Error in attach kernel driver to device " << dev << ": " << libusb_error_name(ret) <<endl;
        libusb_close(handle);
        return ret;
    }else{
        libusb_close(handle);
        return EXIT_SUCCESS;
    }
}

libusb_device *get_device_by_vid_pid(libusb_device **devs, int vid, int pid) {
    libusb_device_descriptor dev_desc;
    int ret;
    ssize_t i = 0;//count device

    while(devs[i]){
        ret = libusb_get_device_descriptor(devs[i], &dev_desc);
        if (ret < 0) {
            qDebug() << "Error Code in libusb_get_config_descriptor of device " << i << " : " << libusb_error_name(ret);
            i++;
        }else {
            if (vid == dev_desc.idVendor && pid == dev_desc.idProduct) {
                qDebug() << "correct";
                return devs[i];
            }else {
                i++;
            }
        }
    }

}

libusb_device *get_device_by_vid_pid_2(libusb_device **devs, int vid, int pid, libusb_context *context) {
    libusb_device_descriptor dev_desc;
    int ret;
    ssize_t i = 0;//count device

    ssize_t list;
    ret = libusb_get_device_list(context, &devs);
    if (ret < 0) {
        qDebug() << "Libusb get device list error: " << libusb_error_name(ret) << endl;
        libusb_free_device_list(devs, 1);
        libusb_exit(context);

    }else{
        list = ret;
    }

    for(i; i < list; i++){
        ret = libusb_get_device_descriptor(devs[i], &dev_desc);
        if (ret < 0) {
            qDebug() << "Error Code in libusb_get_config_descriptor of device " << i << " : " << libusb_error_name(ret);
            i++;
        }else {
            if (vid == dev_desc.idVendor && pid == dev_desc.idProduct) {
                qDebug() << "correct";
                return devs[i];
            }else {
                i++;
            }
        }
    }
}

void device_init(libusb_device **&devs) {
    libusb_device_handle *handle;
    ssize_t i = 0;
    int ret;

    while(devs[i]){
        libusb_open(devs[i], &handle);
        ret = libusb_kernel_driver_active(handle, 0);
        switch (ret) {
        case 0:
            install_device(devs[i]);
            break;
        case 1:
            break;
        case LIBUSB_ERROR_NO_DEVICE:
            qDebug() << "LIBUSB_ERROR_NO_DEVICE";
            break;
        case LIBUSB_ERROR_NOT_SUPPORTED:
            qDebug() << "LIBUSB_ERROR_NOT_SUPPORTED";
            break;
        default:
            break;
        }
       // uninstall_device(devs[i]);
        libusb_close(handle);
        i++;
    }

}

int get_device_list(libusb_device **&devs, libusb_context *&context)
{

    int ret;
    ssize_t i;
    ssize_t list;

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        //nonzero if the running library has the capability, 0 otherwise
        qDebug() << "Hotplug capabilites are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    if (!libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
        //nonzero if the running library has the capability, 0 otherwise
        qDebug() << "Capabilites of detaching kernel driver are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    if (!libusb_has_capability(LIBUSB_CAP_HAS_HID_ACCESS)) {
        //nonzero if the running library has the capability, 0 otherwise
        qDebug() << "Capabilites of detaching kernel driver are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    ret = libusb_get_device_list(context, &devs);
    if (ret < 0) {
        qDebug() << "Libusb get device list error: " << libusb_error_name(ret) << endl;
        libusb_free_device_list(devs, 1);
        libusb_exit(context);
        return EXIT_FAILURE;
    }else{
        list = ret;
    }


    for (i = 0;i < list;i++) {

        qDebug() << i << " ";
        //print_devices(devs[i]);
    }

}













































QString get_device_type(libusb_device* dev) {
    libusb_device_descriptor *dev_desc;
    libusb_config_descriptor *config_desc;
    const libusb_interface *interface;
    //const libusb_interface_descriptor *interface_desc;
    //const libusb_endpoint_descriptor *endpoint_desc;
    //libusb_device_handle *handle = NULL;
    int ret;
    int dev_class, if_class;
    QString type;

    ret = libusb_get_device_descriptor(dev, dev_desc);
    if (ret < 0) {
        qDebug() << "error in detting device descriptor" << libusb_error_name(ret) << endl;
        return "get_device_desc failed";
    }
    dev_class = dev_desc->bDeviceClass;

    ret = libusb_get_config_descriptor(dev, 0, &config_desc);
//    interface = &config_desc->interface[0];
//    if_class = interface->altsetting[0].bInterfaceClass;
    if_class = 1;

    switch (dev_class) {
        case 0:
            switch(if_class){
                case 1:
                    return "Audio音频设备";
                    break;
                case 3:
                    return "HID(Human Interface Device)人机接口设备";
                    break;
                case 5:
                    return "Physical物理设备";
                    break;
                case 6:
                    return "Image图像设备";
                    break;
                case 7:
                    return "Printer打印机";
                    break;
                case 8:
                    return "Mass Storage 大容量存储";
                    break;
                case 10:
                    return "CDC-Data通信设备";
                    break;
                case 11:
                    return "Smart Card智能卡";
                    break;
                case 13:
                    return "Content Security内容安全设备";
                    break;
                case 14:
                    return "Video视频设备";
                    break;
                case 15:
                    return "Personal Healthcare个人健康设备";
                    break;
                case 16:
                    return "Audio/Video Devices声音/视频设备";
                    break;
                case 18:
                    return "USB Type-C Bridge Class";
                    break;
                case 224:
                    return "Wireless Controller无限控制器";
                    break;
                case 254:
                    return "Application Specific特定应用设备";
                    break;
                default:
                    return "Unrecognized Interface";
            }
                break;
        case 2:
            if(if_class == 2){
                return "Communications&CDC通信设备";
            }else {
                return "可能是Communications&CDC通信设备";
            }
            break;
        case 9:
            return "Hub集线器";
            break;
        case 17:
            return "Billboard Device Class广播牌设备";
            break;
        case 220:
            if(if_class == 220){
                return "Diagnostic Device";
            }else {
                return "可能是Diagnostic Device";
            }
            break;
        case 239:
            if(if_class == 239){
                return "Miscellaneous";
            }else {
                return "可能是Miscellaneous";
            }
            break;
        case 255:
            if(if_class == 255){
                return "Vendor Specific厂商自定义设备";
            }else {
                return "可能是Vendor Specific厂商自定义设备";
            }
            break;
        default:
            return "Unrecognized Device";
    }
    return "type";
}


