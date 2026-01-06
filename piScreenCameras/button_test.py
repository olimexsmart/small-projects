from gpiozero import Button
from signal import pause

# GPIO pin where button is connected
BUTTON_PIN = 22

def button_pressed():
    """
    This function is called when the button is pressed.
    Add your custom logic here.
    """
    print("Button pressed!")
    # Add your custom code here

def button_released():
    """
    Optional: This function is called when the button is released.
    """
    print("Button released!")

# Set up the button with pull-up resistor and debounce
# bounce_time is in seconds (0.3 = 300ms)
button = Button(BUTTON_PIN, pull_up=True, bounce_time=0.3)

# Assign the callback function
button.when_pressed = button_pressed
# Optionally handle button release
# button.when_released = button_released

if __name__ == "__main__":
    print(f"Button interrupt set up on GPIO{BUTTON_PIN}")
    print("Waiting for button press... (Ctrl+C to exit)")
    
    try:
        # Keep the program running and wait for events
        pause()
    except KeyboardInterrupt:
        print("\nExiting...")