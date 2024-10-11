#include "capturewindow.h"
#include "ui_capturewindow.h"
#include <QCloseEvent>
#include <QImage>
#include <QFileDialog>
#include <QMessageBox>
#include <QElapsedTimer>
#include <QFile>
#include <QDebug>
#include <QCloseEvent>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>
#include <stdio.h>

#include <cmath>


#define   WIDTH   640
#define   HEIGHT  480
#define   FMT     V4L2_PIX_FMT_YUYV
#define   COUNT   1

capturewindow::capturewindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::capturewindow)
{
    ui->setupUi(this);

    // Connect save button
    connect(ui->saveButton, &QPushButton::clicked, this, &capturewindow::saveFrame);

    ui->imageLabel->setText("No Image Captured Yet");
    // Initialize the camera#include <QVideoFrame>
    isFrameCaptured = false;
    captureFrame();


}

capturewindow::~capturewindow()
{
    delete ui;
}



void yuyvToRgb(unsigned char *yuyv,  QImage &img) {
    unsigned int width = img.width();
    unsigned int height = img.height();

    for (int i = 0; i < width * height * 2; i += 4) {
        int y0 = yuyv[i];
        int u = yuyv[i + 1];
        int y1 = yuyv[i + 2];
        int v = yuyv[i + 3];

        // Convert YUYV to RGB
        int r0 = y0 + 1.402 * (v - 128);
        int g0 = y0 - 0.344136 * (u - 128) - 0.714136 * (v - 128);
        int b0 = y0 + 1.772 * (u - 128);

        int r1 = y1 + 1.402 * (v - 128);
        int g1 = y1 - 0.344136 * (u - 128) - 0.714136 * (v - 128);
        int b1 = y1 + 1.772 * (u - 128);

        r0 = qBound(0, r0, 255);
        g0 = qBound(0, g0, 255);
        b0 = qBound(0, b0, 255);

        r1 = qBound(0, r1, 255);
        g1 = qBound(0, g1, 255);
        b1 = qBound(0, b1, 255);

        img.setPixel((i / 2) % width, (i / 2) / width, qRgb(r0, g0, b0));
        if ((i / 2) + 1 < width * height) {
            img.setPixel(((i / 2) + 1) % width, ((i / 2) + 1) / width, qRgb(r1, g1, b1));
        }
    }
}

void capturewindow::captureFrame()
{
    int ret;
    //unsigned char *buffers[COUNT]; // Memory-mapped buffer

    gettimeofday(&start, NULL);
    fd = open("/dev/video0", O_RDWR);
    if (fd == -1) {
        QMessageBox::warning(this, "Device Error", "Faile to open the device.");
        close();
    }

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = WIDTH;
    format.fmt.pix.height = HEIGHT;
    format.fmt.pix.pixelformat = FMT;
    if (ioctl(fd, VIDIOC_S_FMT, &format) == -1) {
        QMessageBox::warning(this, "V4L2 Error", "Faile to set the format.");
        return;
    }

    struct v4l2_requestbuffers reqbuf;
    reqbuf.count = COUNT;    {
        for(int i = 0; i < COUNT; i++)
            {
                munmap(buffers[i],buff.length);
            }
    }
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &reqbuf) == -1) {
        QMessageBox::warning(this, "V4L2 Error", "Faile to request buffer.");
        return;
    }

    struct v4l2_buffer buff;
    //memset(&buff, 0, sizeof(buff));
    buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buff.memory = V4L2_MEMORY_MMAP;
    for (int i = 0; i < COUNT; i++){
        buff.index = i;
        ret = ioctl(fd, VIDIOC_QUERYBUF, &buff);
        if (-1 == ret){
            break;
        }
        buffers[i] = static_cast<unsigned char*>(mmap(NULL, buff.length, PROT_READ, MAP_SHARED, fd, buff.m.offset));
        if (MAP_FAILED == buffers[i]){
            QMessageBox::warning(this, "V4L2 Error", "Mmap Faile.");
            return;
        }

        ret = ioctl(fd, VIDIOC_QBUF, &buff);    // Queue
        if (-1 == ret)
        {
            QMessageBox::warning(this, "V4L2 Error", "Queue Faile.");
            return ;
        }
    }

    int on = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMON, &on);
    if (-1 == ret)
    {
        QMessageBox::warning(this, "V4L2 Error", "Stream On Faile.");
        return;
    }

    ret = ioctl(fd, VIDIOC_DQBUF, &buff);       // Dequeue
    if (-1 == ret)
    {
        QMessageBox::warning(this, "V4L2 Error", "Dequeue Faile.");
        return;
    }


    //end time: before saving the file
    gettimeofday(&end, NULL);
    delay = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    QString delayText = QString("Frame delay: %1 ms").arg(delay);
    ui->delayLabel->setText(delayText);


    isFrameCaptured = true;
    //Stop the stream
    //int on = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(fd, VIDIOC_STREAMOFF, &on);
    if (-1 == ret) {
        perror("ioctl VIDIOC_STREAM OFF");
        return;
    }

    //fclose(fl);
    ::close(fd);



    // Convert YUYV to RGB using OpenCV
    /*cv::Mat yuyv(480, 640, CV_8UC2, buffers[0]);  // YUYV image with 2 channels //only one image
    cv::Mat rgb;
    cv::cvtColor(yuyv, rgb, cv::COLOR_YUV2RGB_YUYV);  // Convert YUYV to RGB

    // Convert OpenCV Mat to QImage for display in QLabel
    QImage image(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);*/
    QImage image(WIDTH, HEIGHT, QImage::Format_RGB888);


    yuyvToRgb(buffers[0], image);



    // Display the frame
    ui->imageLabel->setPixmap(QPixmap::fromImage(image));

    img = image;
    for(int i = 0; i < COUNT; i++)
    {
        munmap(buffers[i],buff.length);
    }


}

void capturewindow::saveFrame()
{
    if (!isFrameCaptured) {
        QMessageBox::warning(this, "Error", "No frame captured yet!");
        return;
    }

    gettimeofday(&start, NULL);

    // Open file dialog
    //QString fileName = QFileDialog::getSaveFileName(this, "Save Image", "", "Images (*.png *.jpg)");
    //if (fileName.isEmpty())
        //return;

    // Save the current frame


    /*if (image.save(fileName)) {seEvent(QCloseEvent *bar);
        QMessageBox::information(this, "Success", "Image saved successfully!");
    } else {
        QMessageBox::warning(this, "Error", "Failed to save image!");
    }*/
    /*QFile file("./my.yuyv");
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open file:" << file.errorString();
        return;
    }


    file.write(reinterpret_cast<const char*>(buffers[buff.index]), buff.bytesused);


    file.close();*/

    if (!img.save("./my.jpg", "JPG")) {
        qWarning() << "Failed to save image.";
    } else {
        qDebug() << "Image saved successfully at:" << "./my.jpg";
    }




    // Display save time

    gettimeofday(&end, NULL);
    double savetime = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    QString delayText = QString("Image Saved. Frame Save delay: %1 ms").arg(savetime);
    QMessageBox::information(this, "Save Time", delayText);

}


