import time
import requests
import threading
import os
from PIL import Image, ImageDraw, ImageFont
from io import BytesIO

import board
import digitalio
from adafruit_rgb_display import ili9341
from gpiozero import Button

# Read Bearer token from file
with open("camera_token.txt", "r") as f:
    token = f.read().strip()

# Camera settings
CAMERA1_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.garagefuoricam_mainstream_2"
CAMERA2_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.garagedentrocam_mainstream_2"
CAMERA3_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.cama_mainstream"
HEADERS = {
    "Authorization": f"Bearer {token}",
    "Content-Type": "application/json",
}
camera_urls = [CAMERA1_URL, CAMERA2_URL, CAMERA3_URL]

# Configure the SPI display
spi = board.SPI()
dc = digitalio.DigitalInOut(board.D25)
cs = digitalio.DigitalInOut(board.CE0)
reset = digitalio.DigitalInOut(board.D24)

disp = ili9341.ILI9341(
    spi,
    rotation=90,
    cs=cs,
    dc=dc,
    rst=reset,
    baudrate=24000000,
)

# Global state variables
shutdown_pending = False
shutdown_timer = None
locked_camera_index = None
camera_index = 0
running = True

# GPIO pins
BUTTON_SHUTDOWN_PIN = 22
BUTTON_LOCK_PIN = 23

# Helper: fetch image
def fetch_image(camera_url):
    """Fetch image from camera URL"""
    try:
        response = requests.get(camera_url, headers=HEADERS, timeout=5)
        response.raise_for_status()
        image = Image.open(BytesIO(response.content))
        return image.resize((disp.height, disp.width))
    except Exception as e:
        print(f"Failed to fetch image: {e}")
        return None

# Helper: display image
def display_image(image):
    """Display image on screen"""
    if image:
        disp.image(image)

# Helper: display text message
def display_message(text, font_size=20):
    """Display a text message on the screen"""
    image = Image.new("RGB", (disp.height, disp.width), (0, 0, 0))
    draw = ImageDraw.Draw(image)

    try:
        font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", font_size)
    except:
        font = ImageFont.load_default()

    # Get text bounding box for centering
    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    x = (disp.height - text_width) // 2
    y = (disp.width - text_height) // 2

    draw.text((x, y), text, font=font, fill=(255, 255, 255))
    disp.image(image)

# Helper: display loading spinner
def display_spinner():
    """Display a simple loading spinner animation"""
    spinner_chars = ["|", "/", "-", "\\"]
    for _ in range(8):  # Show spinner for ~2 seconds
        for char in spinner_chars:
            display_message(f"Shutting down {char}", font_size=24)
            time.sleep(0.25)


# Shutdown timer callback
def shutdown_timeout():
    """Called when shutdown timer expires without second press"""
    global shutdown_pending
    print("Shutdown cancelled, resuming normal operation")
    shutdown_pending = False

# Button 22 handler (Shutdown)
def button_shutdown_pressed():
    global shutdown_pending, shutdown_timer, running

    if shutdown_pending:
        # Second press within 3 seconds - proceed with shutdown
        print("Shutdown confirmed!")
        if shutdown_timer:
            shutdown_timer.cancel()

        display_spinner()
        running = False
        os.system("sudo shutdown -h now")
    else:
        # First press - start timer
        print("Shutdown requested - press again within 3 seconds to confirm")
        shutdown_pending = True
        display_message("Press again to\nshutdown", font_size=18)

        # Start 3-second timer
        shutdown_timer = threading.Timer(3.0, shutdown_timeout)
        shutdown_timer.start()

# Button 23 handler (Lock/Unlock carousel)
def button_lock_pressed():
    global locked_camera_index, camera_index, camera_urls

    if locked_camera_index is None:
        print("Carousel locked")
        if camera_index == 0:
            locked_camera_index = len(camera_urls) - 1
        else:
            locked_camera_index = camera_index - 1
    else:
        print("Carousel unlocked")
        locked_camera_index = None

# Main carousel loop
def carousel_loop():
    global locked_camera_index, shutdown_pending, running, camera_index, camera_urls
  
    while running:
        # If shutdown is pending, don't update display
        if shutdown_pending:
            time.sleep(0.1)
            continue

        # Fetch and display next camera
        if locked_camera_index is not None:
            image = fetch_image(camera_urls[locked_camera_index])
            print(f"Camera {locked_camera_index} fetched!")
        else:
            # Move to next camera
            camera_index = (camera_index + 1) % len(camera_urls)
            image = fetch_image(camera_urls[camera_index])
            print(f"Camera {camera_index} fetched!")

        if image and not shutdown_pending:
            display_image(image)

# Main entry point
if __name__ == "__main__":
    print("Starting camera carousel with button controls...")
    print(f"Button on GPIO{BUTTON_SHUTDOWN_PIN}: Shutdown control")
    print(f"Button on GPIO{BUTTON_LOCK_PIN}: Lock/Unlock carousel")

    # Set up buttons
    button_shutdown = Button(BUTTON_SHUTDOWN_PIN, pull_up=True, bounce_time=0.3)
    button_lock = Button(BUTTON_LOCK_PIN, pull_up=True, bounce_time=0.3)

    button_shutdown.when_pressed = button_shutdown_pressed
    button_lock.when_pressed = button_lock_pressed

    try:
        # Start the carousel in the main thread
        carousel_loop()
    except KeyboardInterrupt:
        print("\nExiting...")
        running = False
        if shutdown_timer:
            shutdown_timer.cancel()
