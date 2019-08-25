
/*EXTERNAL LIBRARIES*/
#include "mainwindow.h"
#include <QApplication>
#include "QTreeWidget"
#include "pthread.h"
#include <disabler.h>
#include <main_functions.h>

/*INTERNAL LIBRARIES*/


libusb_device **devs;
libusb_context *context = NULL;
map<libusb_device*, QTreeWidgetItem*> dev_item_map;
QList<QTreeWidgetItem*> *items = new QList<QTreeWidgetItem*>;
QTreeWidget* treeWidget;


static int hotplug_callback(struct libusb_context *ctx,struct libusb_device *device,
                            libusb_hotplug_event event, void *user_data);
void *listen_hotplug(void *args);
MainWindow *get_mainwindow(MainWindow *w){
    return w;
}
int hotplug_flush_UI(string op, libusb_device *dev) {

    if(op == "remove") {
        QTreeWidgetItem* item = dev_item_map[dev];
        items->removeOne(item);
        dev_item_map.erase(dev);
        return EXIT_SUCCESS;
    }else if(op == "insert") {
        int *vid_pid;
        vid_pid = get_vid_pid(dev);

        int devs_count = 2;
        int this_vid = vid_pid[0];//必须复制，否则数组内值会因为未知原因溢出
        int this_pid = vid_pid[1];

        QTreeWidgetItem* item = new QTreeWidgetItem;
        map<libusb_device*, QTreeWidgetItem*>::iterator parent_iter;


        qDebug() << this_vid << this_pid;
        QString str = "new";
        qDebug() << str;

        while(devs[devs_count]) {
            devs_count++;
        }

        item->setText(CLMN_DEVICE,  "new");
        item->setText(CLMN_TYPE,    "Unresolved");
        item->setText(CLMN_VID,     QString::fromStdString(to_string(this_vid)));
        item->setText(CLMN_PID,     QString::fromStdString(to_string(this_pid)));
        qDebug() << items->size();
        dev_item_map[dev] = item;
        for(auto iter = dev_item_map.begin(); iter != dev_item_map.end(); iter++) {
            if(iter->first == libusb_get_parent(dev)) {

                iter->second->addChild(item);
                qDebug() << "insert successfully";
            }
        }
        treeWidget->clear();
        treeWidget->insertTopLevelItems(0, *items);
        treeWidget->show();

//        parent_iter = dev_item_map.find(libusb_get_parent(dev));
//        parent_iter->second->addChild(item);
        qDebug() << items->size();
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  MAIN FUNCTION  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow *w = new MainWindow;

    int ret;

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
    libusb_hotplug_flag hp_flag     = static_cast<libusb_hotplug_flag>(0);
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
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  UI  @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
    w->set_devices(devs);
    w->set_maps(dev_item_map);
    w->build_up_tree(devs);
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
     }
    return EXIT_SUCCESS;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@  FUNCTION of MAIN  @@@@@@@@@@@@@@@@@@@@@@@
void *listen_hotplug(void *args) {
    while(1) {
        libusb_handle_events(context);
    }
}
