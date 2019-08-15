#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTreeWidget>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//void MainWindow::set_tree_widget() {
//    //只写结点的实现
//    QList<QTreeWidgetItem *> rootList;


//    ui->treeWidget->insertTopLevelItems(0,rootList);  //将结点插入部件中

//    ui->treeWidget->expandAll(); //全部展开
//}
