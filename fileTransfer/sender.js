const net = require("net")
const fs = require("fs")

let remote_server = process.argv[2];

let socket = remote_server ? net.connect(11861, remote_server) : net.connect(11861);

let fileStream = fs.createReadStream("test.png");

// This will wait until we know the readable stream is actually valid before piping
fileStream.on('open', function () {
  // This just pipes the read stream to the socket, closing it too
  fileStream.pipe(socket);
});



/*
let date = new Date(), size = 0, elapsed;
socket.on('data', chunk => {
  size += chunk.length;
  elapsed = new Date() - date;
  //socket.write(`\r${(size / (1024 * 1024)).toFixed(2)} MB of data was sent. Total elapsed time is ${elapsed / 1000} s`)
  process.stdout.write(`\r${(size / (1024 * 1024)).toFixed(2)} MB of data was sent. Total elapsed time is ${elapsed / 1000} s`);
  ostream.write(chunk);
});
socket.on("end", () => {
  console.log(`\nFinished getting file. speed was: ${((size / (1024 * 1024)) / (elapsed / 1000)).toFixed(2)} MB/s`);
  process.exit();
});
*/



