## libv4l2 loopback

```
$ make
$ ./main -f 0 -r 1 -v /dev/video0
```

```
Usage: ./main [options]
Available options are
 -f <format>    Select frame format
	0 = V4L2_PIX_FMT_YUYV
	1 = V4L2_PIX_FMT_MJPEG
 -h		Print this help screen and exit
 -n		Number of Video buffers (b/w 2 and 32)
 -o <IO method> Select UVC IO method:
	0 = MMAP
	1 = USER_PTR
 -r <resolution> Select frame resolution:
	0 = 360p, VGA
	1 = 720p
 -v device	V4L2 Video Capture device
```
