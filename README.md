# KCF-Tracker
Use ZED and single camera to track
# After connecting to uart
$ sudo chmod 777 /dev/ttyUSB0
# check the number of single camera
$ v4l2-ctl -d /dev/video0(or1or2) --all
# run the code
$ ./sin num ../zed.avi ../single.avi hog multiwindow show...
