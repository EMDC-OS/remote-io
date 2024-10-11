#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "capturewindow.h"
#include <QListWidgetItem>
#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>


#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    QProcess unameProcess;
    unameProcess.start("uname", QStringList() << "-r");
    unameProcess.waitForFinished();
    kernelVersion = unameProcess.readAllStandardOutput().trimmed();

    //qDebug() << "Kernel Version: " << kernelVersion;

    //connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_click);
    connect(ui->DeviceList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_unattachItemDoubleClicked);
    connect(ui->LinkList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_attachItemDoubleClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_searchButton_clicked()
{
    serverIP = ui->IPText->text();


    if (serverIP.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid server IP address.");
        return;
    }
    else{
        QRegularExpression ipRegex("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$");
        QRegularExpressionMatch match = ipRegex.match(serverIP);


        bool valid = true;
        for (int i = 1; i <= 4; ++i) {
            int octet = match.captured(i).toInt();
            if (octet < 0 || octet > 255) {
                valid = false;
                break;
            }
        }

        if(!match.hasMatch() || !valid){
            QMessageBox::warning(this, "Input Error", "Please enter a valid server IP address.");
            return;
        }
    }

    //QProcess unameProcess;
    //unameProcess.start("uname", QStringList() << "-r");
    //unameProcess.waitForFinished();
    //QString kernelVersion = unameProcess.readAllStandardOutput().trimmed();

    QString command = QString("/usr/lib/linux-tools/%1/usbip list -r %2").arg(kernelVersion).arg(serverIP);



   // QString command = QString("/usr/lib/linux-tools/$(uname -r)/usbip list -r %1").arg(serverIP);

    QProcess process;
    process.start(command);
    process.waitForFinished();

    QString result = process.readAllStandardOutput();
    QString error = process.readAllStandardError();

    if(result.isEmpty()){
        QMessageBox::warning(this, "Server Error", "There is no usable device in this server\n");
        return;
    }


    if (!error.isEmpty()) {
        QMessageBox::warning(this, "Command Error", "Error executing usbip command:\n" + error);
        return;
    }

    ui->DeviceList->clear();

    QStringList lines = result.split('\n');
    QString deviceInfo;
    bool foundDevice = false;

    for (const QString &line : lines) {
        if (line.contains(QRegularExpression("\\d+-\\d+:"))) {
            deviceInfo = line.trimmed();
            foundDevice = true;
        } else if (foundDevice && line.trimmed().startsWith(":")) {
            foundDevice = false;
            ui->DeviceList->addItem(deviceInfo);
            //QStringList fields = deviceInfo.split(" : ");

        }
    }

}


void MainWindow::on_unattachItemDoubleClicked(QListWidgetItem *item)
{
    QString clickedText = item->text();

    QString busID = clickedText.section(':', 0, 0).trimmed();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Bind Device",
                                      QString("Do you want to bind this device (Bus ID: %1)?").arg(busID),
                                      QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::Yes){

        if (serverIP.isEmpty()) {
            QMessageBox::warning(this, "Input Error", "Please enter server IP address again.");
            return;
        }


        //sudo /usr/lib/linux-tools/$(uname -r)/usbip attach -r <server IP address> -b <Device bus ID>
        QString command = QString("sudo /usr/lib/linux-tools/%1/usbip attach -r %2 -b %3").arg(kernelVersion).arg(serverIP).arg(busID);



        QProcess process;
        process.start(command);
        process.waitForFinished();

        QString result = process.readAllStandardOutput();
        QString error = process.readAllStandardError();


        if (!error.isEmpty() || !result.isEmpty()) {
            QMessageBox::warning(this, "Command Error", "Error executing usbip command:\n" + error);
            return;
        }else{
            QListWidgetItem *removedItem = ui->DeviceList->takeItem(ui->DeviceList->row(item));

            if (removedItem)
                ui->LinkList->addItem(clickedText);

        }

    }


}


void MainWindow::on_attachItemDoubleClicked(QListWidgetItem *item)
{

    if (access("/dev/video0", F_OK) == -1) {
        QMessageBox::warning(this, "Device Error", "No camera device found.");
        return;
    }else{
        capturewindow *Capturewindow = new capturewindow(this);  // this（MainWindow）
        Capturewindow->setWindowFlags(Qt::Window);
        Capturewindow->setAttribute(Qt::WA_DeleteOnClose);
        Capturewindow->show();


    }

}
