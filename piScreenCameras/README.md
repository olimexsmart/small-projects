# Pi Screen Cameras
A RaspberryPi with a old screen HAT that displays video feeds from surveillance cameras. The video feeds are taken from HomeAssistant. 

### Setup
1. Install `sudo apt install -y   libjpeg-dev   zlib1g-dev   libfreetype6-dev   liblcms2-dev   libopenjp2-7-dev   libtiff5-dev   tk-dev   python3-dev`
2. Be sure that SPI is enabled using `sudo raspi-config`
3. It seems that the recent Raspbian version do not allow to install Python packages system wide. It is necessary to create and activate an environment: 
    ```
    mkdir screen-venv
    python3 -m venv screen-venv/
    source ~/screen-venv/bin/activate
    ```
55. Install system libraries
    ```
    sudo apt install libgfortran5 libopenblas0-pthread libdeflate0 libbz2-1.0 libstdc++6 libzstd1 libjpeg62-turbo libxau6 liblerc4 zlib1g liblcms2-2 libtiff6 libopenjp2-7 liblzma5 libgcc-s1 libwebpdemux2 libfreetype6 libxdmcp6 libwebp7 libwebpmux3 libc6 libxcb1 libsharpyuv0 libbrotli1 libjbig0 libpng16-16t64
    ```
4. Install the required Python packages.
    ```bash
    pip install adafruit-circuitpython-rgb-display requests RPi.GPIO numpy pillow lgpio gpiozero
    ```
5. Copy the Python script in the virtual environment
99. Create a `camera_token.txt` file with just the HomeAssistant HTTP Bearer token
6. Copy the bash script into the home folder (or whatever)
7. Make the bash script executable with `sudo chmod +x run-screen-camera.sh`
8. Create a systemd service: `sudo nano /etc/systemd/system/camera-screen.service`
9. And paste there:
    ```ini
    [Unit]
    Description=Camera Screen Display
    After=network-online.target
    Wants=network-online.target

    [Service]
    Type=simple
    User=olli
    WorkingDirectory=/home/olli
    ExecStart=/home/olli/run-screen-camera.sh
    Restart=always
    RestartSec=5
    # Give access to GPIO & SPI
    Environment=PYTHONUNBUFFERED=1

    [Install]
    WantedBy=multi-user.target
    ```
10. Then actually enable it with:
    ```
    sudo systemctl enable camera-screen.service
    sudo systemctl start camera-screen.service
    sudo systemctl status camera-screen.service

    # To stop:
    sudo systemctl stop camera-screen.service
    ```

