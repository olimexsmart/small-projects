const net = require("net")
const fs = require("fs")
const http = require("http")

var fileName = "test.png"

// Preparing data to be sent
var stats = fs.statSync(fileName)
var fileSizeInBytes = stats["size"]
console.log("SIZE " + fileSizeInBytes)
let data = JSON.stringify({
  filename: 'received.png',
  size: fileSizeInBytes
})

// Create HTTP POST request
let httpClient = http.request({
  host: 'localhost',
  port: 11861,
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': data.length
  }
}, res => {

  // Intercept response data
  res.on('data', d => { 
    let socket = net.connect(parseInt(d), 'localhost')

    socket.on('error', error => {
      console.log(error)
    })
    
    // Open file to send
    let fileStream = fs.createReadStream(fileName);

    // Send file
    fileStream.on('open', function () {
      // This just pipes the read stream to the socket, closing it too
      fileStream.pipe(socket);
    });
  })
})

httpClient.on('error', error => {
  console.error(error)
})


// Send request
httpClient.write(data)
httpClient.end()




