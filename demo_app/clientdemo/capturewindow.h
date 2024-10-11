#ifndef CAPTUREWINDOW_H
#define CAPTUREWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
//#include <QTimer>
//#include <QTime>
#include <QProcess>
#include <QImage>
#include <sys/time.h> // for delay calculation

// V4L2 headers for camera access
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

namespace Ui {
class capturewindow;
}

class capturewindow : public QWidget
{
    Q_OBJECT

public:
    explicit capturewindow(QWidget *parent = nullptr);
    ~capturewindow();



private slots:
    void captureFrame();   // Capture and display frame
    void saveFrame();      // Save current frame and calculate save time


private:
    Ui::capturewindow *ui;
    QLabel *imageLabel;    // For image preview
    QLabel *delayLabel;    // To show delay
    QPushButton *saveButton;
    QVBoxLayout *mainLayout;

    int fd;                // File descriptor for camera

    struct v4l2_buffer buff;
    struct timeval start, end;
    QString fileName;
    unsigned char *buffers[1]; // Memory-mapped buffer
    //QTimer *timer;         // For frame capturing
    double delay;          // Captured frame delay
    bool isFrameCaptured;  // Check if a frame is captured
    QImage img;
};

#endif // CAPTUREWINDOW_H
