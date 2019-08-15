#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTreeWidget>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    set_tree_widget();
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::set_tree_widget() {
    //只写结点的实现
    QList<QTreeWidgetItem *> rootList;

    QTreeWidgetItem *imageItem1 = new QTreeWidgetItem;   //添加第一个父节点
    imageItem1->setText(0,tr("图像1"));
    rootList.append(imageItem1);

    QTreeWidgetItem *imageItem1_1 = new QTreeWidgetItem(imageItem1,QStringList(QString("Band1"))); //添加子节点
    imageItem1->addChild(imageItem1_1);

    QTreeWidgetItem *imageItem2 = new QTreeWidgetItem;   //添加第二个父节点
    imageItem2->setText(0,tr("图像2"));
    rootList.append(imageItem2);

    QTreeWidgetItem *imageItem2_1 = new QTreeWidgetItem(imageItem2,QStringList(QString("Band1")));  //添加子节点
    QTreeWidgetItem *imageItem2_2 = new QTreeWidgetItem(imageItem2,QStringList(QString("Band2")));
    imageItem2->addChild(imageItem2_1);
    imageItem2->addChild(imageItem2_2);

    ui->treeWidget->insertTopLevelItems(0,rootList);  //将结点插入部件中

    ui->treeWidget->expandAll(); //全部展开
}
