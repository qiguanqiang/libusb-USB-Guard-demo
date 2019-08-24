#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 1024, 768);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_devices(libusb_device **devs) {
    this->devs = devs;
}

void MainWindow::set_maps(map<libusb_device *, QTreeWidgetItem *> dev_item_map) {
    this->dev_item_map = dev_item_map;
}

void MainWindow::build_up_tree(libusb_device** devs) {

    treeWidget->setColumnCount(4);
    QStringList labels;
    labels.append("Device");
    labels.append("Type");
    labels.append("Vendor ID");
    labels.append("Product ID");
    treeWidget->setHeaderLabels(labels);
    treeWidget->setGeometry(0, 0, 1024, 768);
    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    arange_tree(devs);
    treeWidget->expandAll();// must be placed behind 'insertTopLevel'
    treeWidget->show();
}

void MainWindow::arange_tree(libusb_device **devs) {
    /* for device returning NULL parent pointer: be parent
     * for device returning a non-NULL device, go get a parent
     * for device did not find the parent in UI, go to stack and match later
     */
    int i = 0;
    int *vid_pid;

    while(devs[i]) {
       // tmp_dev = libusb_get_parent(devs[i]);
        QTreeWidgetItem *tmp_item = new QTreeWidgetItem;
        vid_pid = get_vid_pid(devs[i]);

        int this_vid = vid_pid[0];//必须复制，否则数组内值会因为未知原因变
        int this_pid = vid_pid[1];
//        QString str = get_device_type(devs[i]);
//        qDebug() << str;
        tmp_item->setText(CLMN_DEVICE,  QString::fromStdString(to_string(i)));
        tmp_item->setText(CLMN_TYPE,    "Unresolved");
        tmp_item->setText(CLMN_VID,     QString::fromStdString(to_string(this_vid)));
        tmp_item->setText(CLMN_PID,     QString::fromStdString(to_string(this_pid)));

        if(libusb_get_parent(devs[i]) == NULL) {
            items.append(tmp_item);
        }
        /*build device-item map*/
        dev_item_map[devs[i]] = tmp_item;
        i++;
    }
    for(auto iter = dev_item_map.begin(); iter != dev_item_map.end(); iter++) {
        libusb_device *this_parent;
        this_parent = libusb_get_parent(iter->first);

        if(this_parent != NULL) {
            auto iter_parent = dev_item_map.begin();
            while(iter_parent != dev_item_map.end()) {
                if(iter_parent->first == this_parent) {
                    iter_parent->second->addChild(iter->second);
                }
                iter_parent++;
            }
        }

    }
    treeWidget->insertTopLevelItems(0, items);
    //click_item(treeWidget);
}

void MainWindow::click_item() {

    QTreeWidgetItem* this_item = treeWidget->currentItem();  //**获取当前被点击的节点
    if(this_item == NULL/* || this_item->parent() == NULL*/) {
        return;           //右键的位置在空白位置右击或者点击的是顶层item
    }

    libusb_device* this_dev;
    for(auto iter = dev_item_map.begin(); iter != dev_item_map.end(); iter++) {
        if(iter->second == this_item) {
            this_dev = iter->first;
        }
    }

    //创建action
    int *vid_pid;
    vid_pid = get_vid_pid(this_dev);
    disabler dis;
    if(dis.is_device_disabled(vid_pid[0], vid_pid[1]) == DISABLED) {
        QMenu *menu = new QMenu(treeWidget);
        QAction enable_item(QString::fromLocal8Bit("启用"), treeWidget);

        menu->addAction(&enable_item);
        menu->exec(QCursor::pos());
    }else {                                            //ENABLED
        libusb_device_handle **handle;
        libusb_open(this_dev, handle);

        if(libusb_kernel_driver_active(*handle, 0) == 0) { //not using
            QAction install_item(QString::fromLocal8Bit("安装"), treeWidget);
            QAction disable_item(QString::fromLocal8Bit("禁用"), treeWidget);
            QMenu *menu = new QMenu(treeWidget);

            menu->addAction(&install_item);
            menu->addAction(&disable_item);
            menu->exec(QCursor::pos());
        }else if(libusb_kernel_driver_active(*handle, 0) == 1) { //using
            QAction uninstall_item(QString::fromLocal8Bit("卸载"), treeWidget);
            QAction disable_item(QString::fromLocal8Bit("禁用"), treeWidget);
            QMenu *menu = new QMenu(treeWidget);
            //connect(disable_item, SIGNAL(triggered(bool)), this_item, SLOT(NULL));

            menu->addAction(&uninstall_item);
            menu->addAction(&disable_item);
            menu->exec(QCursor::pos());
        }
    }

}

int MainWindow::hotplug_flush_UI(string op, libusb_device *dev) {
    QTreeWidgetItem* item = dev_item_map[dev];
    int *vid_pid;
    map<libusb_device*, QTreeWidgetItem*>::iterator parent_iter;

    if(op == "remove") {
        items.removeOne(item);
        dev_item_map.erase(dev);
        return EXIT_SUCCESS;
    }else if(op == "insert") {
        vid_pid = get_vid_pid(dev);
        int devs_count = 2;
        int this_vid = vid_pid[0];//必须复制，否则数组内值会因为未知原因溢出
        int this_pid = vid_pid[1];

        qDebug() << this_vid << this_pid;
        QString str = "new";
        qDebug() << str;

        while(devs[devs_count]) {
            devs_count++;
        }

        item->setText(CLMN_DEVICE,  QString::fromStdString(to_string(devs_count)));
        item->setText(CLMN_TYPE,    "Unresolved");
        item->setText(CLMN_VID,     QString::fromStdString(to_string(this_vid)));
        item->setText(CLMN_PID,     QString::fromStdString(to_string(this_pid)));
        dev_item_map[dev] = item;

        parent_iter = dev_item_map.find(libusb_get_parent(dev));
        parent_iter->second->insertChild(0,item);
        qDebug() << "insert successfully";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

void MainWindow::register_sig_slot() {
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(right_click_slot(QPoint)));
}

void MainWindow::right_click_slot(QPoint) {
    QTreeWidgetItem* this_item = treeWidget->currentItem();
    if(this_item == NULL) {
        return;
    }
    libusb_device* this_dev;
    for(auto iter = dev_item_map.begin(); iter != dev_item_map.end(); iter++) {
        if(iter->second == this_item) {
            this_dev = iter->first;
        }
    }

    //创建action
    int *vid_pid;
    vid_pid = get_vid_pid(this_dev);
    disabler dis;
    QMenu *menu = new QMenu(treeWidget);

    if(dis.is_device_disabled(vid_pid[0], vid_pid[1]) == DISABLED) {
        QAction *enable_item = new QAction("启用", treeWidget);

        menu->addAction(enable_item);
        menu->move(cursor().pos());
        menu->show();
    }else {                                            //ENABLED
        libusb_device_handle **handle;
        libusb_open(this_dev, handle);

        if(libusb_kernel_driver_active(*handle, 0) == 0) { //not using
            QAction *install_item = new QAction("安装", treeWidget);
            QAction *disable_item = new QAction("禁用", treeWidget);
            menu->addAction(install_item);
            menu->addAction(disable_item);
            menu->move(cursor().pos());
            menu->show();

        }else if(libusb_kernel_driver_active(*handle, 0) == 1) { //using
            QAction *uninstall_item = new QAction("卸载", treeWidget);
            QAction *disable_item   = new QAction("禁用", treeWidget);
            //connect(disable_item, SIGNAL(triggered(bool)), this_item, SLOT(NULL));

            menu->addAction(uninstall_item);
            menu->addAction(disable_item);
            menu->move(cursor().pos());
            menu->show();
        }
    }
}
