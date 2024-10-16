#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_searchButton_clicked();
    void on_unattachItemDoubleClicked(QListWidgetItem *item);
    void on_attachItemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QString serverIP;
    QString kernelVersion;
};
#endif // MAINWINDOW_H
