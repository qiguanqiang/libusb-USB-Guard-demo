/*EXTERNAL LIBRARIES*/
#include "mainwindow.h"
#include <QApplication>
#include "QTreeWidget"
#include "pthread.h"

/*INTERNAL LIBRARIES*/
#include "libs.h"
#include "disabler.h"

#define CLMN_DEVICE 0
#define CLMN_TYPE   1
#define CLMN_VID    2
#define CLMN_PID    3

libusb_device **devs;
libusb_context *context = NULL;

void test_qt_tree(libusb_device **devs);
void arange_tree(QTreeWidget *&treeWidget, libusb_device **devs);
string get_dev_type(libusb_device *dev);
void device_init(libusb_device **&devs);
int uninstall_device(libusb_device *dev);
int install_device(libusb_device *dev);
int get_device_list(libusb_device **&devs, libusb_context *&context);
void print_devices(libusb_device *dev);
int *get_vid_pid(libusb_device *dev);
libusb_device *get_device_by_vid_pid(libusb_device **devs, int vid, int pid);
libusb_device *get_device_by_vid_pid_2(libusb_device **devs, int vid, int pid, libusb_context *context);
//static int hotplug_callback(struct libusb_context *ctx, struct libusb_device *device,
//    libusb_hotplug_event event, void *user_data);

static int hotplug_callback(struct libusb_context *ctx,struct libusb_device *device,
                            libusb_hotplug_event event, void *user_data) {
    if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        int *vid_pid;
        int ret;

        vid_pid = get_vid_pid(device);
        disabler dis;
        if(dis.is_device_disabled(vid_pid[0], vid_pid[1]) == DISABLED) {
            ret = uninstall_device(device);
            if(ret < 0) {
                return -1;
            }
        }
     }else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        libusb_free_device_list(devs, 1);
        get_device_list(devs, context);
     }
    return EXIT_SUCCESS;
}
void *listen_hotplug(void *args) {
    while(1) {
        libusb_handle_events(context);
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //    MainWindow w;
    //    w.show();
    int ret;
    libusb_device *dev;

    pthread_t listener;


    ret = libusb_init(&context);
    if (ret < 0) {
        cout << "failed to initialise libusb: " << libusb_error_name(ret) << endl;
        return EXIT_FAILURE;
    }
    get_device_list(devs,context);
    device_init(devs);

    libusb_hotplug_event hp_event   = static_cast<libusb_hotplug_event> (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT);
    int hp_vid                      = LIBUSB_HOTPLUG_MATCH_ANY;
    int hp_pid                      = LIBUSB_HOTPLUG_MATCH_ANY;
    int hp_dev_class                = LIBUSB_HOTPLUG_MATCH_ANY;
    libusb_hotplug_flag hp_flag     = static_cast<libusb_hotplug_flag>(1);
    void *hp_user_data;
    libusb_hotplug_callback_handle cb_handle = NULL;

    int result_Register = -2;
    result_Register = libusb_hotplug_register_callback(context, hp_event, hp_flag, hp_vid, hp_pid, hp_dev_class, hotplug_callback, &hp_user_data, &cb_handle);
    if (result_Register != (int)LIBUSB_SUCCESS) {
        cout << "Resigter hotplug_callback failed";
        return -1;
    } else {
       cout << "Resigter hotplug_callback successfully";
    }

    ret = pthread_create(&listener, NULL, listen_hotplug, NULL);
    if(ret != 0) {
        cout << "Creating pthread failed" << endl;
    }

    test_qt_tree(devs);

//    while(1) {
//        libusb_handle_events(context);
//    }
    //libusb_handle_events_timeout(context, 0);
    a.exec();

    libusb_hotplug_deregister_callback(context, cb_handle);
    libusb_free_device_list(devs,1);
    libusb_exit(context);

    pthread_exit(NULL);
    return 0;
}

int get_device_list(libusb_device **&devs, libusb_context *&context)
{

    int ret;
    ssize_t i;
    ssize_t list;

    if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
        //nonzero if the running library has the capability, 0 otherwise
        cout << "Hotplug capabilites are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    if (!libusb_has_capability(LIBUSB_CAP_SUPPORTS_DETACH_KERNEL_DRIVER)) {
        //nonzero if the running library has the capability, 0 otherwise
        cout << "Capabilites of detaching kernel driver are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }
    if (!libusb_has_capability(LIBUSB_CAP_HAS_HID_ACCESS)) {
        //nonzero if the running library has the capability, 0 otherwise
        cout << "Capabilites of detaching kernel driver are not supported on this platform" << endl;
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    ret = libusb_get_device_list(context, &devs);
    if (ret < 0) {
        cout << "Libusb get device list error: " << libusb_error_name(ret) << endl;
        libusb_free_device_list(devs, 1);
        libusb_exit(context);
        return EXIT_FAILURE;
    }else{
        list = ret;
    }


    for (i = 0;i < list;i++) {

        cout << i << " ";
        //print_devices(devs[i]);
    }

}

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
        cout << "error in detting device descriptor" << libusb_error_name(ret) << endl;
        return;
    }

    cout << "Number of POS configs is " << dev_desc.bNumConfigurations << endl;
    cout << "Device Vendor id): " << dev_desc.idVendor << endl;
    cout << "Product ID: " << dev_desc.idProduct << endl;
    cout << "Class ID: " << dev_desc.bDeviceClass << endl;

    ret = libusb_get_config_descriptor(dev, 0, &config_desc);
    if (ret == LIBUSB_ERROR_NOT_FOUND) {
        /*0 on success
        LIBUSB_ERROR_NOT_FOUND if the configuration does not exist
        another LIBUSB_ERROR code on error*/
        cout << "The configuration does not exist" << endl;
    }else if(ret < 0){
        cout << "Error Code in libusb_get_config_descriptor: " << libusb_error_name(ret);
    }else{
        cout << "Interface: " << config_desc->bNumInterfaces << endl;
    }
    cout << "*********************************************************" << endl;
}
int *get_vid_pid(libusb_device *dev) {
    libusb_device_descriptor dev_desc;
    int ret;
    int vid_pid[2];

    ret = libusb_get_device_descriptor(dev, &dev_desc);
    if (ret < 0) {
        cout << "error in detting device descriptor" << libusb_error_name(ret) << endl;
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
        cout << "Error in detach kernel driver to device " << dev << ": " << libusb_error_name(ret) <<endl;
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
        cout << "Error in attach kernel driver to device " << dev << ": " << libusb_error_name(ret) <<endl;
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
            cout << "Error Code in libusb_get_config_descriptor of device " << i << " : " << libusb_error_name(ret);
            i++;
        }else {
            if (vid == dev_desc.idVendor && pid == dev_desc.idProduct) {
                cout << "correct";
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
        cout << "Libusb get device list error: " << libusb_error_name(ret) << endl;
        libusb_free_device_list(devs, 1);
        libusb_exit(context);

    }else{
        list = ret;
    }

    for(i; i < list; i++){
        ret = libusb_get_device_descriptor(devs[i], &dev_desc);
        if (ret < 0) {
            cout << "Error Code in libusb_get_config_descriptor of device " << i << " : " << libusb_error_name(ret);
            i++;
        }else {
            if (vid == dev_desc.idVendor && pid == dev_desc.idProduct) {
                cout << "correct";
                return devs[i];
            }else {
                i++;
            }
        }
    }
}

//static int hotplug_callback(struct libusb_context *ctx, struct libusb_device *device,
//    libusb_hotplug_event event, void *user_data) {
//    //uint8_t deviceAddress = libusb_get_device_address(device);

//    if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
//        int *vid_pid;
//        int ret;

//        libusb_free_device_list(devs, 1);
//        get_device_list(devs, ctx);

//        vid_pid = get_vid_pid(device);
//        disabler dis;
//        if(dis.is_device_disabled(vid_pid[0], vid_pid[1]) == DISABLED) {
//           ret = uninstall_device(device);
//           if(ret < 0) {
//               return -1;
//           }
//         }
//    } else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
//        libusb_free_device_list(devs, 1);
//        get_device_list(devs, ctx);
//    }
////    int i=1;
////    i++;

//}

/*static int LIBUSB_CALL hotplug_callback(
    libusb_context *context, libusb_device *dev,
    libusb_hotplug_event event, void *user_data) {
    int ret;
    //enum libusb_hotplug_event {
    //     LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED = 0x01,
    //     LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT = 0x02 }
    if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
        int *vid_pid;

        libusb_free_device_list(devs, 1);
        get_device_list(devs, context);

        vid_pid = get_vid_pid(dev);
        disabler dis;
        if(dis.is_device_disabled(vid_pid[0], vid_pid[1]) == DISABLED) {
           ret = uninstall_device(dev);
           if(ret < 0) {
               return -1;
           }
         }
     }else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        libusb_free_device_list(devs, 1);
        get_device_list(devs, context);
     }
    return EXIT_SUCCESS;
}*/


void mute(libusb_device *dev) {

}

void device_init(libusb_device **&devs) {
    libusb_device_handle *handle;
    ssize_t i;
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
            cout << "LIBUSB_ERROR_NO_DEVICE";
            break;
        case LIBUSB_ERROR_NOT_SUPPORTED:
            cout << "LIBUSB_ERROR_NOT_SUPPORTED";
            break;
        default:
            break;
        }
       // uninstall_device(devs[i]);
        libusb_close(handle);
        i++;
    }

}

void test_qt_tree(libusb_device** devs) {
    QTreeWidget *treeWidget = new QTreeWidget();
    treeWidget->setColumnCount(4);
    QStringList labels;
    labels.append("Device");
    labels.append("Type");
    labels.append("Vendor ID");
    labels.append("Product ID");
    treeWidget->setHeaderLabels(labels);
    treeWidget->setGeometry(0, 0, 1024, 768);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

//    QList<QTreeWidgetItem *> items;
//    //treeWidget->insertTopLevelItems(0, items);
//    QTreeWidgetItem *item = new QTreeWidgetItem;
//    item->setText(0, "hello");
//    QTreeWidgetItem *c = new QTreeWidgetItem;
//    c->setText(0, "world");
//    item->addChild(c);
//    items.append(item);

//    QTreeWidgetItem *item2 = new QTreeWidgetItem;
//    item2->setText(0, "hello");
//    items.append(item2);


    arange_tree(treeWidget, devs);
    treeWidget->expandAll();// must be placed behind 'insertTopLevel'
    treeWidget->show();
}

void arange_tree(QTreeWidget *&treeWidget, libusb_device **devs) {
    /* for device returning NULL parent pointer: be parent
     * for device returning a non-NULL device, go get a parent
     * for device did not find the parent in UI, go to stack and match later
     */
    int i;
    int ret;
    libusb_device *tmp_dev = NULL;
    QList<QTreeWidgetItem *> items;
    vector<libusb_device **> no_parent_list;

    while(devs[i]) {
        tmp_dev = libusb_get_parent(devs[i]);
        if(tmp_dev == NULL) {
            QTreeWidgetItem *tmp_item;
            int *vid_pid;
            vid_pid = get_vid_pid(devs[i]);

            QString qstr = QString::fromStdString(to_string(i));
            tmp_item->setText(CLMN_DEVICE, qstr);
            tmp_item->setText(CLMN_TYPE, "NOT_FOUND");
            qstr = QString::fromStdString(to_string(vid_pid[0]));
            tmp_item->setText(CLMN_VID, qstr);
            qstr = QString::fromStdString(to_string(vid_pid[1]));
            tmp_item->setText(CLMN_PID, qstr);
            items.append(tmp_item);
            i++;
        }
    }
    treeWidget->insertTopLevelItems(0, items);
}

string get_dev_type(libusb_device *dev) {
    return "NOT_FOUND";
}

void click_item() {
    QTreeWidgetItem* curItem=treeWidget->currentItem();  //**获取当前被点击的节点
        if(curItem == NULL || curItem->parent() == NULL)
            return;           //右键的位置在空白位置右击或者点击的是顶层item

        //创建一个action
        QAction deleteItem(QString::fromLocal8Bit("&删除"),this);
        connect(&deleteItem, SIGNAL(triggered()), this, SLOT(deleteItem()));
        QPoint pos;
        //创建一个菜单栏
        QMenu menu(treeWidget);
        menu.addAction(&deleteItem);
        menu.exec(QCursor::pos());  //在当前鼠标位置显示
}









