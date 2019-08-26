#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QMenu>
#include <QDebug>
#include <libs.h>
#include <disabler.h>
#include <main_functions.h>

#define CLMN_DEVICE 0
#define CLMN_TYPE   1
#define CLMN_VID    2
#define CLMN_PID    3

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private slots:
    void right_click_slot(QPoint);

private:


public:
    libusb_device **devs;
    QTreeWidget* treeWidget = new QTreeWidget(this);
    QList<QTreeWidgetItem*> *items = new QList<QTreeWidgetItem*>;
    map<libusb_device*, QTreeWidgetItem*> dev_item_map;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /*@@@  my functions  @@@*/

    //init and set values
    void init_tree();
    void set_devices(libusb_device **devs);
    void set_maps(map<libusb_device*, QTreeWidgetItem*> &dev_item_map);

    //functional
    void build_up_tree(libusb_device **devs);
    void arange_tree(libusb_device **devs);
    void click_item();
    int  hotplug_flush_UI(string op, libusb_device *dev);

    void register_sig_slot();
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
