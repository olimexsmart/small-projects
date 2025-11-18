import time
import requests
from PIL import Image
from io import BytesIO

import board
import digitalio
from adafruit_rgb_display import ili9341  # For PID1601

# === 1️⃣ Read Bearer token from file ===
with open("camera_token.txt", "r") as f:
    token = f.read().strip()

# === 2️⃣ Camera settings ===
CAMERA1_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.garagefuoricam_mainstream_2"
CAMERA2_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.garagedentrocam_mainstream_2"
CAMERA3_URL = "http://192.168.42.18:8123/api/camera_proxy/camera.cama_mainstream"
HEADERS = {
    "Authorization": f"Bearer {token}",
    "Content-Type": "application/json",
}

# === 3️⃣ Configure the SPI display ===
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

# === 4️⃣ Helper: fetch and display image ===
def fetch_and_display(camera_url):
    print("Fetching image...")
    try:
        response = requests.get(camera_url, headers=HEADERS, timeout=5)
        response.raise_for_status()
    except Exception as e:
        print("Failed to fetch image:", e)
        return

    # Convert the response into an image
    try:
        image = Image.open(BytesIO(response.content))
        # Resize to display resolution
        image = image.resize((disp.height, disp.width))
        # Send to screen
        disp.image(image)
        print("Image displayed!")
    except Exception as e:
        print("Failed to decode/display image:", e)

# === 5️⃣ Main loop ===
if __name__ == "__main__":
    while True:
        fetch_and_display(CAMERA1_URL)
        fetch_and_display(CAMERA2_URL)
        fetch_and_display(CAMERA3_URL)
        #time.sleep(5)  # refresh every 5 seconds