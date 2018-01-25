# 3D-KCF-Tracker
Use ZED and single camera to track
# After connecting to uart
$ sudo chmod 777 /dev/ttyUSB0
# check the number of single camera
$ v4l2-ctl -d /dev/video0(or1or2) --all
# run the code
$ ./sin num ../zed.avi ../single.avi hog multiwindow show...

# Relative Tracker

## Describe
This project aims to track two objects, then return their relative image coordinates and optical parallax.

## New function
* We us zed camera for image grab an disparity (optical parallax) measurement.
* We add UART to send message
		+ The Port name is fixed as `ttyUSB0`, and the bound rate is `115200`. If you use TX1/TX2, you should alter `ttyUSB0` to `ttyS0` in `runtracker.cpp`.

## Running instructions

The executable "RT" should be called as:   
```
./RT [OPTION_1] [OPTION_2] [...]
```
You can run `RT` without any option, i.e., you use the default setting and needn't record video. But note that if there are options, `OPTION_1` should be `NONE` or path to record video, e.g., `../data/test.avi`.

Other Options available:   

gray - Use raw gray level features.   
hog - Use HOG features.   
lab - Use Lab colorspace features. This option will also enable HOG features by default.   
singlescale - Performs single-scale detection, using a variable-size window.  Default `Fasle`.   
fixed_window - Keep the window size fixed when in single-scale mode (multi-scale always used a fixed window). Default `True`.  
no_show - Show the results in a window. Default `Fasle`.
