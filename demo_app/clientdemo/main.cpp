#include "mainwindow.h"

#include <QApplication>
#include <QProcess>
#include <QMessageBox>
#include <QCoreApplication>
#include <QApplication>

bool executeModprobeCommand()
{
    QProcess process;
    //QString command = "pkexec modprobe vhci-hcd";
    QString command = "sudo modprobe vhci-hcd";

    process.start(command);
    process.waitForFinished();

    QString error = process.readAllStandardError();
    if (!error.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Application Execution Failed.\nFailed to execute 'modprobe vhci-hcd':\n" + error);
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QMessageBox greeting;

    greeting.setText("This application requires a sudo permission for execution. Please enter your password.");
    greeting.exec();

    if (!executeModprobeCommand()) {
        return 1;
    }

    MainWindow w;
    w.show();
    return a.exec();
}
