let bluetoothDevice
let gattServer
let service
let characteristic
let notifyCharacteristic

const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b'
const CHARACTERISTIC_UUID = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'

async function connectToDevice() {
  try {
    clearLogs()

    logMessage("Requesting Bluetooth Device...")
    bluetoothDevice = await navigator.bluetooth.requestDevice({
      acceptAllDevices: true,
      optionalServices: [SERVICE_UUID]
    })

    logMessage("Connecting to GATT Server...")
    gattServer = await bluetoothDevice.gatt.connect()

    logMessage("Getting Service...")
    service = await gattServer.getPrimaryService(SERVICE_UUID)

    logMessage("Getting Read/Write Characteristic...")
    characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID)

    logMessage("Getting Notification Characteristic...")
    notifyCharacteristic = await service.getCharacteristic(CHARACTERISTIC_UUID)

    enableButtons()
    logMessage("Connected successfully.")
  } catch (error) {
    logMessage("Error: " + error)
  }
}

async function disconnectFromDevice() {
  try {
    if (bluetoothDevice) {
      await bluetoothDevice.gatt.disconnect()
      logMessage("Disconnected.")
    }
  } catch (error) {
    logMessage("Disconnect Error: " + error)
  }
}

async function readCharacteristic() {
  try {
    if (!characteristic) return logMessage("Characteristic not available.")
    const value = await characteristic.readValue()
    logMessage("Read Value: " + new TextDecoder().decode(value))
  } catch (error) {
    logMessage("Read Error: " + error)
  }
}

async function writeCharacteristic(val) {
  if (typeof val !== 'string')
    val = "CIAO"
  try {
    if (!characteristic) return logMessage("Characteristic not available.")
    const encoder = new TextEncoder()
    await characteristic.writeValue(encoder.encode(val))
    logMessage("Value written successfully.")
  } catch (error) {
    logMessage("Write Error: " + error)
  }
}

async function startNotifications() {
  try {
    await writeCharacteristic("NOTIFY_ON")
    if (!notifyCharacteristic) return logMessage("Notify characteristic not available.")
    await notifyCharacteristic.startNotifications()
    notifyCharacteristic.addEventListener('characteristicvaluechanged', handleNotifications)
    logMessage("Notifications started.")
  } catch (error) {
    logMessage("Notify Error: " + error)
  }
}

async function stopNotifications() {
  try {
    await writeCharacteristic("NOTIFY_OFF")
    if (!notifyCharacteristic) return logMessage("Notify characteristic not available.")
    await notifyCharacteristic.stopNotifications()
    notifyCharacteristic.removeEventListener('characteristicvaluechanged', handleNotifications)
    logMessage("Notifications stopped.")
  } catch (error) {
    logMessage("Stop Notify Error: " + error)
  }
}

function handleNotifications(event) {
  let receivedValue = new TextDecoder().decode(event.target.value)
  logMessage("Notification received: " + receivedValue)
}

function logMessage(message) {
  const logDiv = document.getElementById("log")
  const logItem = document.createElement("div")
  logItem.className = "log-item"
  logItem.textContent = new Date().toLocaleTimeString() + " - " + message
  logDiv.append(logItem)
}

function clearLogs() {
  const logDiv = document.getElementById("log")
  logDiv.innerHTML = ""
}

function enableButtons() {
  document.getElementById("write").disabled = false
  document.getElementById("read").disabled = false
  document.getElementById("notifyOn").disabled = false
  document.getElementById("notifyOff").disabled = false
}

// Attach event listeners
document.getElementById("connect").addEventListener("click", connectToDevice)
document.getElementById("disconnect").addEventListener("click", disconnectFromDevice)
document.getElementById("read").addEventListener("click", readCharacteristic)
document.getElementById("write").addEventListener("click", writeCharacteristic)
document.getElementById("notifyOn").addEventListener("click", startNotifications)
document.getElementById("notifyOff").addEventListener("click", stopNotifications)
