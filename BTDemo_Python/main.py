import asyncio
from bleak import BleakClient, BleakScanner

# Replace this with your ESP32's Bluetooth address
ESP32_BLUETOOTH_ADDRESS = "F4:12:FA:74:D9:31"
CHAR_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
i = 0
data_received_event = asyncio.Event()

def notification_handler(sender, data):
    global i
    i = i + 1
    print(f"NOTIFY: {data.decode('utf-8')}")
    
    if(i > 3):
        data_received_event.set()

# Function to connect and print text
async def connect_and_print():
    try:
        # Scan for available Bluetooth devices
        print("Scanning for Bluetooth devices...")
        devices = await BleakScanner.discover()

        # Check if the desired ESP32 is among the discovered devices
        for device in devices:
            print(f"Found device: {device.name} - {device.address}")
            
            if device.address == ESP32_BLUETOOTH_ADDRESS:
                print(f"ESP32 found: {device.name}")

                # Connect to the ESP32
                async with BleakClient(device.address) as client:
                    print("Connected to ESP32")
                    
                    for service in client.services:
                        print(f"Service: {service.uuid}")
                        for characteristic in service.characteristics:
                            print(f"  Characteristic: {characteristic.uuid} | Properties: {characteristic.properties}")
                    # WRITE TEST
                    for i in range(3):
                        # Send the text
                        text_to_print = "Ciao_" + str(i)
                        await client.write_gatt_char(CHAR_UUID, text_to_print.encode('utf-8'))
                        print(f"WRITE: {text_to_print}")
                    # READ TEST
                    print("READ: " + (await client.read_gatt_char(CHAR_UUID)).decode('utf-8'))
                    # NOTIFY TEST
                    await client.start_notify(CHAR_UUID, notification_handler)
                    await client.write_gatt_char(CHAR_UUID, "NOTIFY_ON".encode('utf-8'))
                    data_received_event.clear()
                    await data_received_event.wait()
                    await client.write_gatt_char(CHAR_UUID, "NOTIFY_OFF".encode('utf-8'))
                    #await client.stop_notify(CHAR_UUID)
                    

                print("END")
                return

        print("ESP32 not found!")
    
    except Exception as e:
        print(f"Error: {e}")

# Run the async function
if __name__ == "__main__":
    asyncio.run(connect_and_print())
