import subprocess
import threading
import time
from pathlib import Path

import requests
from PIL import Image, ImageDraw, ImageFont, ImageOps
from io import BytesIO

import board
import digitalio
from adafruit_rgb_display import ili9341
from gpiozero import Button

BASE_DIR = Path(__file__).resolve().parent
TOKEN_FILE = BASE_DIR / "camera_token.txt"

# Read the token relative to this script, so systemd can start it from any
# working directory.  Do not log the token if the file is missing or invalid.
try:
    token = TOKEN_FILE.read_text(encoding="utf-8").strip()
except OSError as exc:
    raise SystemExit(f"Cannot read {TOKEN_FILE}: {exc}") from exc
if not token:
    raise SystemExit(f"{TOKEN_FILE} is empty")

# Camera settings
CAMERA1_URL = "http://192.168.42.16:8123/api/camera_proxy/camera.garagefuoricam_mainstream_2"
CAMERA2_URL = "http://192.168.42.16:8123/api/camera_proxy/camera.garagedentrocam_mainstream_2"
CAMERA3_URL = "http://192.168.42.16:8123/api/camera_proxy/camera.cama_mainstream"
HEADERS = {
    "Authorization": f"Bearer {token}",
    "Content-Type": "application/json",
}
camera_urls = [CAMERA1_URL, CAMERA2_URL, CAMERA3_URL]
CAMERA_INTERVAL_SECONDS = 5
REQUEST_TIMEOUT = (3, 8)

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
current_camera_index = 0
running = True
display_lock = threading.Lock()
http = requests.Session()
http.headers.update(HEADERS)

# GPIO pins
BUTTON_SHUTDOWN_PIN = 22
BUTTON_LOCK_PIN = 23

# Helper: fetch image
def fetch_image(camera_url):
    """Fetch image from camera URL"""
    try:
        response = http.get(camera_url, timeout=REQUEST_TIMEOUT)
        response.raise_for_status()
        with Image.open(BytesIO(response.content)) as source:
            # Home Assistant may return a different aspect ratio per camera.
            # Fit and crop instead of stretching the image across the TFT.
            image = source.convert("RGB")
            return ImageOps.fit(
                image,
                (disp.height, disp.width),
                method=Image.Resampling.LANCZOS,
                centering=(0.5, 0.5),
            )
    except Exception as e:
        print(f"Failed to fetch image: {e}")
        return None

# Helper: display image
def display_image(image):
    """Display image on screen"""
    if image:
        with display_lock:
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
    bbox = draw.multiline_textbbox((0, 0), text, font=font, spacing=4, align="center")
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]

    x = (disp.height - text_width) // 2
    y = (disp.width - text_height) // 2

    draw.multiline_text((x, y), text, font=font, fill=(255, 255, 255), spacing=4, align="center")
    with display_lock:
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
    global shutdown_pending, shutdown_timer
    print("Shutdown cancelled, resuming normal operation")
    shutdown_pending = False
    shutdown_timer = None

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
        subprocess.run(["sudo", "shutdown", "-h", "now"], check=False)
    else:
        # First press - start timer
        print("Shutdown requested - press again within 3 seconds to confirm")
        shutdown_pending = True
        display_message("Press again to\nshutdown", font_size=18)

        # Start a 3-second confirmation timer. A daemon timer must not keep
        # the process alive if the program is stopped by systemd.
        shutdown_timer = threading.Timer(3.0, shutdown_timeout)
        shutdown_timer.daemon = True
        shutdown_timer.start()

# Button 23 handler (Lock/Unlock carousel)
def button_lock_pressed():
    global locked_camera_index, current_camera_index

    if locked_camera_index is None:
        print("Carousel locked")
        locked_camera_index = current_camera_index
    else:
        print("Carousel unlocked")
        locked_camera_index = None

# Main carousel loop
def carousel_loop():
    global locked_camera_index, shutdown_pending, running
    global camera_index, camera_urls, current_camera_index
  
    last_images = {}

    while running:
        # If shutdown is pending, don't update display
        if shutdown_pending:
            time.sleep(0.1)
            continue

        # Fetch and display next camera
        if locked_camera_index is not None:
            index = locked_camera_index
        else:
            # Move to next camera
            index = camera_index
            camera_index = (camera_index + 1) % len(camera_urls)
        current_camera_index = index

        image = fetch_image(camera_urls[index])
        if image is not None:
            last_images[index] = image
            print(f"Camera {index} fetched")
        elif index in last_images:
            image = last_images[index]
            print(f"Camera {index} unavailable; showing last frame")
        else:
            display_message(f"Camera {index + 1}\nunavailable", font_size=20)

        if image and not shutdown_pending:
            display_image(image)

        # Avoid hammering Home Assistant and give the display time to settle.
        deadline = time.monotonic() + CAMERA_INTERVAL_SECONDS
        while running and time.monotonic() < deadline:
            time.sleep(0.1)

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
    finally:
        button_shutdown.close()
        button_lock.close()
        http.close()
