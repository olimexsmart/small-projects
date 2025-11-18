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
4. Install the required Python packages.
    ```bash
    pip install adafruit-circuitpython-rgb-display requests RPi.GPIO
    pip install --no-cache-dir pillow
    ```
5. Copy the Python script in the virtual environment
99. Create a `camera_token.txt` file with just the HomeAssistant HTTP Bearer token
6. Copy the bash script into the home folder (or whatever)
7. Make the bash script executable with `sudo chmod +x launch-script.sh`
8. Create a systemd service: `sudo nano /etc/systemd/system/camera-screen.service`
9. And paste there:
    ```ini
    [Unit]
    Description=Camera Screen Display
    After=network-online.target
    Wants=network-online.target

    [Service]
    Type=simple
    User=pi
    WorkingDirectory=/home/pi
    ExecStart=/home/pi/run-camera.sh
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
    ```

