#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QMenu>
#include <QDebug>
class MainWindow : public QMainWindow
{
    Q_OBJECT
private slots:
    void right_click_slot(QPoint);
    void test();
public:
    QTreeWidget* tree = new QTreeWidget(this);
    QList<QTreeWidgetItem*> items;
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTreeWidget *get_tree();
    void register_sig_slot();

};

#endif // MAINWINDOW_H
