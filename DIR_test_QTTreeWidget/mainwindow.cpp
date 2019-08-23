#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setGeometry(0, 0, 1024, 768);

    tree->setColumnCount(0);
    tree->setHeaderLabel("title");
    tree->setGeometry(0, 0, 1024, 768);
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    for(int i = 0; i < 10; i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem;
        item->setText(0, "test");
        items.append(item);
    }
    tree->insertTopLevelItems(0, items);
    tree->expandAll();
    tree->show();
}

MainWindow::~MainWindow()
{

}

void MainWindow::register_sig_slot() {
    tree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tree, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(right_click_slot(QPoint)));

}

void MainWindow::right_click_slot(QPoint) {
    QMenu *menu = new QMenu(this);
    QAction *act = new QAction("nothing", this);
    connect(act, SIGNAL(triggered()), this, SLOT(test()));
    menu->addAction(act);
    menu->move(cursor().pos());
    menu->show();

}
void MainWindow::test() {
    qDebug() << "ok";
}
