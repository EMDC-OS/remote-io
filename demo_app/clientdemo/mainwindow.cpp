#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QListWidgetItem>
#include <QProcess>
#include <QMessageBox>
#include <QRegularExpression>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::on_searchButton_click);
    connect(ui->DeviceList, &QListWidget::itemDoubleClicked, this, &MainWindow::onItemDoubleClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_searchButton_clicked()
{
    QString serverIP = ui->IPText->text();


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

    QProcess unameProcess;
    unameProcess.start("uname", QStringList() << "-r");
    unameProcess.waitForFinished();
    QString kernelVersion = unameProcess.readAllStandardOutput().trimmed();

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


void MainWindow::onItemDoubleClicked(QListWidgetItem *item)
{
    QString clickedText = item->text();

    QMessageBox::information(this, "Device Info", "You clicked on:\n" + clickedText);
}
