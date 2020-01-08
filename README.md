# esp32-mjpeg-ipcam
The goal is to turn cheap AI Thinker ESP32-CAM module into simple reliable mjpeg IP camera usable with surveillance software such as https://motion-project.github.io/

This project is based on https://github.com/yoursunny/esp32cam Arduino library / WifiCam example. 

In the past I tried to use https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/Camera/CameraWebServer/CameraWebServer.ino but the ESP module was freezing every like 1-3 days, which is bad for surveillance camera. It also lacks the authentication and other configuration options.

## Features
- authentication (basic, motion does not support digest auth for mjpeg cams)
- framerate / jpeg quality / port / resolution / url & more configuration
- removed useless code such as static image capture

## Usage
- install ESP32 board support and https://github.com/yoursunny/esp32cam Arduino library into your Arduino IDE
- set board type to "AI Thinker ESP32-CAM" in Arduino IDE -> Tools -> Board
- open esp32-mjpeg-ip-camera.ino, change WiFi SSID and password to your own, compile & upload
- open Serial monitor and restart the ESP, wait until WiFi connected
- open http://<ip_of_you_esp>/cam.mjpeg in web browser on computer connected to the same WiFi (you can get the IP from Arduino's Serial monitor)
- enter esp32 / pass32 as your username / password, you should get picture from your camera
- you can modify stream username / password / url / port / resolution / etc in the sketch

## Motion configuration
I suggest you to install latest version of motion as there are usually quite old versions of it in most linux distribution's repos.
- https://github.com/Motion-Project/motion/releases
- https://motion-project.github.io/motion_build.html

I'm using following configuration to connect motion software to my camera (192.168.88.111 is the IP of my ESP32-CAM):
```
camera_name esp32
rotate 180
width 800
height 600
framerate 10
netcam_url mjpeg://192.168.88.111:80/stream
netcam_userpass esp32:pass32
netcam_keepalive off
netcam_tolerant_check off
netcam_use_tcp on
auto_brightness off
threshold 1000
threshold_tune off
noise_level 32
noise_tune on
despeckle_filter EedDl
minimum_motion_frames 3
pre_capture 0
post_capture 100
picture_output best
picture_quality 65
picture_type jpeg
movie_output on
movie_quality 40
movie_codec mp4
movie_duplicate_frames on
snapshot_interval 900
locate_motion_mode on
locate_motion_style box
text_right %Y-%m-%d\n%T
text_left ESP-cam-01
text_changes on
text_event %Y%m%d%H%M%S
text_double off
target_dir /mnt/motion/cam-ESP32
snapshot_filename snapshots/%Y-%m-%d--%H-%M-%S-snapshot
picture_filename %Y-%m-%d/%H-%M-%S-%q
movie_filename %Y-%m-%d/%H-%M-%S-%v
timelapse_filename %Y%m%d-timelapse
stream_port 8183
stream_quality 65
stream_motion on
stream_maxrate 10
stream_localhost off
stream_auth_method 0

```
## Hints
- this camera is able to serve the video stream to ONE CLIENT ONLY, keep this in mind when testing - this also means that stream opened in your browser is BLOCKING other clients (this also apply to motion) from connecting to you camera (you can easily verify this when you try to open the video in two tabs at once - only the first one gets the video)
- configure your router to assing the same IP address to your ESP32-CAM every time it gets rebooted / connected (different IP is also usually assigned when the router itself is rebooted for example because of power outage if this is not done)
- to disable authentication comment out this block in handleMjpeg()
```
  if(!server.authenticate(streamUsername, streamPassword)) {
    Serial.println(F("STREAM auth required, sending request"));
    return server.requestAuthentication(BASIC_AUTH, streamRealm, authFailResponse);
  }   
```

## Todo
- possibility to easily configure static IP
- possibility to allow connection from specific IP only (something like very simple firewall)
- possibility to control onboard LED (most probably using mqtt which also allows reporting of WiFi signal and other integrations like homeassistant)


