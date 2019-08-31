

/* 包含一个main函数调用 Qt 的 GUI 制作图形界面和后端操作及写在这个文件
    的其他函数（因参数需要不能放在其他文件。包括一个设备热插拔监听回调函
    数、一个线程回调函数和一个热插拔刷新 GUI 的函数。 */


/*EXTERNAL LIBRARIES*/
#include "mainwindow.h"
#include <QApplication>
#include "QTreeWidget"
#include "pthread.h"
#include "iostream"
#include "libusb.h"
#include "map"

using namespace std;

/*INTERNAL LIBRARIES*/
#include <disabler.h>
#include <main_functions.h>


libusb_device **devs;
libusb_context *context = NULL;
map<libusb_device*, QTreeWidgetItem*> dev_item_map;
QList<QTreeWidgetItem*> *items;
QTreeWidget* treeWidget;


static int hotplug_callback(struct libusb_context *ctx,struct libusb_device *device,
                            libusb_hotplug_event event, void *user_data);
void *listen_hotplug(void *args);
int hotplug_flush_UI(string op, libusb_device *dev);



//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  MAIN FUNCTION  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow *w = new MainWindow;
    a.setApplicationName("Ri-Guard USB Manager");

    int ret;

    pthread_t listener;

    ret = libusb_init(&context);
    if (ret < 0) {
        qDebug() << "failed to initialise libusb: " << libusb_error_name(ret) << endl;
        return EXIT_FAILURE;
    }
    get_device_list(devs,context);
    device_init(devs);

    libusb_hotplug_event hp_event   = static_cast<libusb_hotplug_event> (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT);
    int hp_vid                      = LIBUSB_HOTPLUG_MATCH_ANY;
    int hp_pid                      = LIBUSB_HOTPLUG_MATCH_ANY;
    int hp_dev_class                = LIBUSB_HOTPLUG_MATCH_ANY;
    libusb_hotplug_flag hp_flag     = static_cast<libusb_hotplug_flag>(0);
    void *hp_user_data;
    libusb_hotplug_callback_handle cb_handle = NULL;

    int result_Register = -2;
    result_Register = libusb_hotplug_register_callback(context, hp_event, hp_flag, hp_vid, hp_pid, hp_dev_class, hotplug_callback, &hp_user_data, &cb_handle);
    if (result_Register != (int)LIBUSB_SUCCESS) {
        qDebug() << "Resigter hotplug_callback failed";
        return -1;
    } else {
       qDebug() << "Resigter hotplug_callback successfully";
    }

    ret = pthread_create(&listener, NULL, listen_hotplug, NULL);
    if(ret != 0) {
        qDebug() << "Creating pthread failed" << endl;
    }
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  UI  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    w->set_devices(devs);
//    disabler dis;
//    dis.enable_record(8053,2307);
    w->build_up_tree(devs);
    dev_item_map = w->dev_item_map;
    w->register_sig_slot();
    items = w->items;
    treeWidget = w->treeWidget;
    w->show();
    a.exec();

    libusb_hotplug_deregister_callback(context, cb_handle);
    libusb_free_device_list(devs,1);
    libusb_exit(context);

    pthread_exit(NULL);
    return 0;
}


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  FUNCTION of MAIN  @@@@@@@@@@@@@@@@@@@@@@@

/* 在线程中的回调函数，以阻塞方式监听热插拔，与 Qt 的 a.exe() 相冲突故
    放在线程中 */
void *listen_hotplug(void *args) {
    while(1) {
        libusb_handle_events(context);
    }
}

/* 热插拔监听回调函数中刷新 GUI 的函数。主要包括向 QTreeWidget 
    插入 item 或者删除 item。
    参数：操作符 op，代表移除或者添加；与 item 对应的设备：dev
    返回值： 成功则返回EXIT_SUCCESS；失败则返回EXIT_FAILURE */
int hotplug_flush_UI(string op, libusb_device *dev) {

    if(op == "remove") {
        map<libusb_device*, QTreeWidgetItem*>::iterator dev_iter, parent_iter;
        dev_iter    = dev_item_map.find(dev);
        parent_iter = dev_item_map.find(libusb_get_parent(dev));
        QTreeWidgetItem* parent_item = parent_iter->second;
        parent_item->removeChild(dev_iter->second);
        dev_item_map.erase(dev);

        return EXIT_SUCCESS;
    }else if(op == "insert") {
        int *vid_pid;
        vid_pid = get_vid_pid(dev);

        int devs_count = 2;
        int this_vid = vid_pid[0];//必须复制，否则数组内值会因为未知原因溢出
        int this_pid = vid_pid[1];

        QTreeWidgetItem* item = new QTreeWidgetItem;

        qDebug() << this_vid << this_pid;

        while(devs[devs_count]) {
            devs_count++;
        }

        item->setText(CLMN_DEVICE,  "new");
        item->setText(CLMN_TYPE,    "Unresolved");
        item->setText(CLMN_VID,     QString::fromStdString(to_string(this_vid)));
        item->setText(CLMN_PID,     QString::fromStdString(to_string(this_pid)));
        qDebug() << items->size();
        dev_item_map[dev] = item;
        int i = 1;
        for(auto iter = dev_item_map.begin(); iter != dev_item_map.end(); iter++) {
            if(iter->first == libusb_get_parent(dev)) {
                iter->second->addChild(item);
                qDebug() << "insert successfully";
            }
            qDebug() << dev_item_map.size() << "size";
            qDebug() << i << "TIMES";
            i++;
        }

        qDebug() << "FLushed";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

/* libusb指定的热插拔回调函数，参数必须是这个形式。设备热插拔后会自动获取
    此设备的信息，可以在回调函数中进行相关操作。
    这里的操作有：若有设备插入，判断是否是被禁用设备，是则卸载。刷新 GUI
                若有设备离开，刷新 GUI */
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

        hotplug_flush_UI("insert", device);
     }else if (event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
        libusb_free_device_list(devs, 1);
        get_device_list(devs, context);
        hotplug_flush_UI("remove", device);

        qDebug() << "LEAVED!";
     }
    return EXIT_SUCCESS;
}
