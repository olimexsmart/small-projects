import time
import board
import digitalio
from PIL import Image, ImageDraw
from adafruit_rgb_display import ili9341

# --- SPI and pin setup (typical for PID1601) ---
spi = board.SPI()
dc = digitalio.DigitalInOut(board.D25)  # Data/Command
cs = digitalio.DigitalInOut(board.CE0)  # Chip select
reset = digitalio.DigitalInOut(board.D24)  # Reset
display = ili9341.ILI9341(spi, cs=cs, dc=dc, rst=reset, baudrate=32000000)

# --- Test 1: Fill screen with solid colors ---
for color in [(255, 0, 0), (0, 255, 0), (0, 0, 255)]:  # Red, Green, Blue
    image = Image.new("RGB", (display.width, display.height), color)
    display.image(image)
    time.sleep(1)

# --- Test 2: Draw text and shapes ---
image = Image.new("RGB", (display.width, display.height), "black")
draw = ImageDraw.Draw(image)
draw.rectangle((10, 10, 230, 310), outline="white", width=3)
draw.text((50, 150), "Hello TFT!", fill="yellow")
display.image(image)
print("Test image displayed.")