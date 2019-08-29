#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/* 这个类是 Qt 前端类 */

#include <QMainWindow>
#include <QTreeWidget>
#include <QMenu>
#include <QDebug>
#include "libusb.h"
#include "map"
#include "QtSql/QSqlDatabase"
#include "QtSql/QSqlRecord"
#include "QtSql/QSqlQuery"

#include <disabler.h>
#include <main_functions.h>

/* 关于树控件表头的宏定义 */
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
    /* 一些槽函数 */
    void right_click_slot(QPoint);  //右键点击槽函数，生成菜单（安装、卸载、禁用、启用）
    void install_slot();            //点击菜单选项槽函数，执行安装设备操作
    void uninstall_slot();          //点击菜单选项槽函数，执行卸载设备操作
    void disable_slot();            //点击菜单选项槽函数，执行禁用设备操作
    void enable_slot();             //点击菜单选项槽函数，执行启用设备操作

private:
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
public:
    libusb_device **devs;
    QTreeWidget* treeWidget = new QTreeWidget(this);
    QList<QTreeWidgetItem*> *items = new QList<QTreeWidgetItem*>;
    map<libusb_device*, QTreeWidgetItem*> dev_item_map;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    /*@@@  my functions  @@@*/

    //init and set values
    void init_tree();                           //初始化设备树，与操作系统连接的设备
    void set_devices(libusb_device **devs);     //获取设备列表的函数
    void set_maps(map<libusb_device*, QTreeWidgetItem*> &dev_item_map);//获取设备和 item 映射的map

    //functional
    void build_up_tree(libusb_device **devs);   //组建和显示树
    void arange_tree(libusb_device **devs);     //规划树，主要是设备间的父子关系
    void click_item();                          //右键点击 item 的信号函数
    void register_sig_slot();                   //注册信号和槽函数的函数
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
