#include <stdio.h>
#include <string.h>

#include "v4l2.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [options]\n", argv0);
    fprintf(stderr, "Available options are\n");
    fprintf(stderr,
            " -f <format>    Select frame format\n\t"
            "0 = V4L2_PIX_FMT_YUYV\n\t"
            "1 = V4L2_PIX_FMT_MJPEG\n");
    fprintf(stderr, " -h		Print this help screen and exit\n");
    fprintf(stderr, " -n		Number of Video buffers (b/w 2 and 32)\n");
    fprintf(stderr,
            " -o <IO method> Select UVC IO method:\n\t"
            "0 = MMAP\n\t"
            "1 = USER_PTR\n");
    fprintf(stderr,
            " -r <resolution> Select frame resolution:\n\t"
            "0 = HEIGHT1p, VGA (WIDTH1xHEIGHT1)\n\t"
            "1 = 720p, (WIDTH2xHEIGHT2)\n");
    fprintf(stderr, " -v device	V4L2 Video Capture device\n");
}

int main(int argc, char *argv[])
{
    struct v4l2_device *vdev;
    struct timeval tv;

    struct v4l2_format fmt;
    char *v4l2_devname = "/dev/video1";

    fd_set fdsv;
    int ret, opt, nfds;

    /* Frame format/resolution related params. */
    int default_format = 0;     /* V4L2_PIX_FMT_YUYV */
    int default_resolution = 0; /* VGA 360p */
    int nbufs = 2;              /* Ping-Pong buffers */

    enum io_method uvc_io_method = IO_METHOD_USERPTR;

    while ((opt = getopt(argc, argv, "f:h:n:o:r:v:")) != -1) {
        switch (opt) 
        {
        case 'f':
            if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                usage(argv[0]);
                return 1;
            }

            default_format = atoi(optarg);
            break;

        case 'h':
            usage(argv[0]);
            return 1;

        case 'n':
            if (atoi(optarg) < 2 || atoi(optarg) > 32) {
                usage(argv[0]);
                return 1;
            }

            nbufs = atoi(optarg);
            printf("Number of buffers requested = %d\n", nbufs);
            break;

        case 'o':
            if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                usage(argv[0]);
                return 1;
            }

            uvc_io_method = atoi(optarg);
            printf("IO method requested is %s\n", (uvc_io_method == IO_METHOD_MMAP) ? "MMAP" : "USER_PTR");
            vdev->io = uvc_io_method;
            break;

        case 'r':
            if (atoi(optarg) < 0 || atoi(optarg) > 1) {
                usage(argv[0]);
                return 1;
            }

            default_resolution = atoi(optarg);
            break;

        case 'v':
            v4l2_devname = optarg;
            break;

        default:
            printf("Invalid option '-%c'\n", opt);
            usage(argv[0]);
            return 1;
        }
    }

    /*
     * Try to set the default format at the V4L2 video capture
     * device as requested by the user.
    */
    CLEAR(fmt);

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = (default_resolution == 0) ? WIDTH1 : WIDTH2;
    fmt.fmt.pix.height = (default_resolution == 0) ? HEIGHT1 : HEIGHT2;
    fmt.fmt.pix.sizeimage = (default_format == 0) ? (fmt.fmt.pix.width * fmt.fmt.pix.height * 2)
                                                    : (fmt.fmt.pix.width * fmt.fmt.pix.height * 1.5);
    fmt.fmt.pix.pixelformat = (default_format == 0) ? V4L2_PIX_FMT_YUYV : V4L2_PIX_FMT_MJPEG;
    fmt.fmt.pix.field = V4L2_FIELD_ANY;

    /* Open the V4L2 device. */
    ret = v4l2_open(&vdev, v4l2_devname, &fmt);
    if (vdev == NULL || ret < 0)
        return 1;

    vdev->v4l2_devname = v4l2_devname;

    /* UVC - V4L2 integrated path */
    vdev->nbufs = nbufs;

    if (IO_METHOD_MMAP == vdev->io) {
        /*
         * Ensure that the V4L2 video capture device has already some
         * buffers queued.
         */
        v4l2_reqbufs(vdev, vdev->nbufs);
    }

    while (1) 
    {
        FD_ZERO(&fdsv);

        FD_SET(vdev->v4l2_fd, &fdsv);

        v4l2_qbuf_mmap(vdev);
        v4l2_start_capturing(vdev);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        nfds = vdev->v4l2_fd;
        ret = select(nfds + 1, &fdsv, NULL, NULL, &tv);

        if (-1 == ret) {
            printf("select error %d, %s\n", errno, strerror(errno));
            if (EINTR == errno)
                continue;
            break;
        }

        if (0 == ret) {
            printf("select timeout\n");
            break;
        }

        if (FD_ISSET(vdev->v4l2_fd, &fdsv))
            v4l2_process_data(vdev);

        cv::Mat out_img;

        // if (default_format == 0) {
            // Decode YUYV
            cv::Mat img = cv::Mat(cv::Size((default_resolution == 0) ? WIDTH1 : WIDTH2, (default_resolution == 0) ? HEIGHT1 : HEIGHT2), CV_8UC2, vdev->mem[1].start);
            cv::cvtColor(img, out_img, cv::COLOR_YUV2RGB_YVYU);
        // }
        // else {
            // Decode MJPEG (OpenCV4 Only)
            // unsigned int a_size = (default_resolution == 0) ? (WIDTH1*HEIGHT1*3) : (WIDTH2*HEIGHT2*3);
            // cv::_InputArray pic_arr(buffer, a_size);
            // out_img = cv::imdecode(pic_arr, cv::IMREAD_UNCHANGED);
        // }

        // imwrite("output.jpg", out_img);

        imshow("view", out_img);
        cv::waitKey(1);
    }

    if (vdev->is_streaming) {
        /* Stop V4L2 streaming... */
        v4l2_stop_capturing(vdev);
        v4l2_uninit_device(vdev);
        v4l2_reqbufs(vdev, 0);
        vdev->is_streaming = 0;
    }

    v4l2_close(vdev);

    return 0;
}
