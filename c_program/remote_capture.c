
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>                      
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <netdb.h> 
#include <linux/videodev2.h>     
#include <sys/time.h>           

#define   WIDTH   640                      
#define   HEIGHT  480                      
#define   FMT     V4L2_PIX_FMT_YUYV        
#define   COUNT   1                       


int main(int argc, char **argv)
{	

    struct timeval start,end;
    unsigned char *buffers[COUNT];            // buffer address
	int ret, i;
	int fd;
	gettimeofday(&start, NULL);
	//printf("Start time: %ld seconds, %ld microseconds\n", start.tv_sec, start.tv_usec);
	/* 1st: open camera device */
	fd = open("/dev/video0", O_RDWR);       
	if (-1 == fd)
	{
		perror("open /dev/video0");
		return -1;
	}

	/* 2nd: set format */
	struct v4l2_format format;
	format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // capture one frame
	format.fmt.pix.width = WIDTH;               
	format.fmt.pix.height = HEIGHT;             
	format.fmt.pix.pixelformat = FMT;           
	ret = ioctl(fd, VIDIOC_S_FMT, &format);     // set format
	if (-1 == ret)
	{
		perror("ioctl VIDIOC_S_FMT");
		close(fd);
		return -1;
	}
	
	/* 3rd: check for success */
	ret = ioctl(fd, VIDIOC_G_FMT, &format);     // Get
	if (-1 == ret)
	{
		perror("ioctl VIDIOC_G_FMT");
		close(fd);
		return -1;
	}
	if (format.fmt.pix.pixelformat == FMT)
	{
		printf("ioctl VIDIOC_S_FMT sucessful\n");
	}
	else
	{
		printf("ioctl VIDIOC_S_FMT failed\n");
	}

	/* 4th：ask camera driver to request the buffer storing image data */
	struct v4l2_requestbuffers reqbuf;
	reqbuf.count = COUNT;                       
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;  
	reqbuf.memory = V4L2_MEMORY_MMAP;           // for memory mapping
	ret = ioctl(fd, VIDIOC_REQBUFS, &reqbuf);
	if (-1 == ret)
	{
		perror("ioctl VIDIOC_REQBUFS");
		close(fd);
		return -1;
	}

	/* 5th：check all the buffer and do the mmap */
	struct v4l2_buffer buff;
	buff.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buff.memory = V4L2_MEMORY_MMAP;
	for (i = 0; i < COUNT; i++)
	{
		buff.index = i;
		ret = ioctl(fd, VIDIOC_QUERYBUF, &buff);
		if (-1 == ret)                  
		{
			break;
		}

		//rintf("buf[%d]: len = %d offset: %d\n", i, buff.length, buff.m.offset);
		
		/* map each buffer to the current process */
		buffers[i] = mmap(NULL, buff.length, PROT_READ, MAP_SHARED, fd, buff.m.offset);
		if (MAP_FAILED == buffers[i])            
		{
			perror("mmap failed");
			return -1;
		}

		/* add mmaped buffer to camera driver image data queue */
		ret = ioctl(fd, VIDIOC_QBUF, &buff);    // Queue
		if (-1 == ret)
		{
			perror("queue failed");
			return -1;
		}
	}

	/* 6th: start record */
	int on = V4L2_BUF_TYPE_VIDEO_CAPTURE;       
	ret = ioctl(fd, VIDIOC_STREAMON, &on);      
	if (-1 == ret)
	{
		perror("ioctl VIDIOC_STREAM ON");
		return -1;
	}

	/* 7th: dequeue*/
	ret = ioctl(fd, VIDIOC_DQBUF, &buff);       // Dequeue
	if (-1 == ret)
	{
		perror("ioctl dequeue");
		return -1;
	}

	//end time: before saving the file
	gettimeofday(&end, NULL); 
	
	
    FILE *fl;
	fl = fopen("./my.yuyv", "w");
	if (NULL == fl)
	{
		fprintf(stderr, "open write file failed.");
	}
	fwrite(buffers[buff.index], buff.bytesused, 1, fl);
    

	fclose(fl);  

	//Stop the stream
	ret = ioctl(fd, VIDIOC_STREAMOFF, &on);      
    if (-1 == ret) {
        perror("ioctl VIDIOC_STREAM OFF");
        return -1;
    }
	                             
	close(fd);  

        
    double delay = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0; 
    printf("Total delay: %.2f ms\n", delay);           

                      
	return 0;
}
