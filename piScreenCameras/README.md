# Pi Screen Cameras
A RaspberryPi with a old screen HAT that displays video feeds from surveillance cameras. The video feeds are taken from HomeAssistant. 

## Setup

1. Enable SPI with `sudo raspi-config`, then install the prerequisites:

   ```bash
   sudo apt install -y python3-venv python3-dev libjpeg-dev zlib1g-dev \
     libfreetype6-dev libopenjp2-7-dev libtiff-dev
   ```

2. Create the virtual environment and install the Python dependencies:

   ```bash
   python3 -m venv /home/olli/screen-venv
   /home/olli/screen-venv/bin/pip install \
     adafruit-circuitpython-rgb-display requests pillow gpiozero
   ```

3. Keep `display_camera_buttons.py`, `run-screen-camera.sh`, and
   `camera_token.txt` together (for example in `/home/olli/piScreenCameras`).
   Put only the Home Assistant token in the token file and protect it:

   ```bash
   chmod 600 /home/olli/piScreenCameras/camera_token.txt
   chmod +x /home/olli/piScreenCameras/run-screen-camera.sh
   ```

4. Update `run-screen-camera.sh` if your username or project path differs.
   Create `/etc/systemd/system/camera-screen.service` containing:

   ```ini
   [Unit]
   Description=Camera Screen Display
   After=network-online.target
   Wants=network-online.target

   [Service]
   Type=simple
   User=olli
   WorkingDirectory=/home/olli/piScreenCameras
   ExecStart=/home/olli/piScreenCameras/run-screen-camera.sh
   Restart=always
   RestartSec=5
   Environment=PYTHONUNBUFFERED=1

   [Install]
   WantedBy=multi-user.target
   ```

5. Enable and start the service:

   ```bash
   sudo systemctl daemon-reload
   sudo systemctl enable camera-screen.service
   sudo systemctl start camera-screen.service
   sudo systemctl status camera-screen.service
   ```

To stop it, run `sudo systemctl stop camera-screen.service`.

The carousel changes camera every five seconds. Press GPIO22 once to request
shutdown and again within three seconds to confirm. Press GPIO23 to lock the
currently displayed camera, or press it again to resume the carousel.
