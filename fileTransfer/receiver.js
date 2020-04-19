const net = require("net")
const fs = require("fs")
const http = require("http")
const nodeDiskInfo = require('node-disk-info'); //=> Use this when install as a dependency


let httpServer = http.createServer((request, response) => {

    // Collect data from POST body 
    let body = [];
    request.on('error', (err) => {
        console.error(err);
    }).on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        const data = JSON.parse(Buffer.concat(body).toString())
        const name = data.filename

        // Check if there is enough space to receive the file
        // TODO we need to know where we are saving the file first
        const size = data.size
        const disks = nodeDiskInfo.getDiskInfoSync();

        // Create the server that will listen for the incoming file
        const server = net.createServer()

        // Write on file received data
        server.on("connection", socket => {
            let fileStream = fs.createWriteStream(name);
            let progressChecker = null
            fileStream.on('ready', () => {
                socket.pipe(fileStream);

                progressChecker = setInterval((fileStreamToCheck) => {
                    progress = fileStreamToCheck.bytesWritten / size
                    // This needs to be updated in GUI
                    console.log(progress)
                }, 200, fileStream)
            })

            socket.on("end", () => {
                clearInterval(progressChecker)
                server.close(() => { });
            })
        })

        server.on('error', error => {
            console.log(error)
        })

        server.listen({
            port: 0
        }, () => {
            // When the server is initialized
            // send the HTTP response with the port choosen
            response.writeHead(200);
            response.end(server.address().port.toString());

            // Set a callback to close server if no file were sent
            setTimeout((serverToClose) => {
                serverToClose.getConnections((err, count) => {
                    if (count == 0 && serverToClose.listening)
                        serverToClose.close()
                })
            }, 3000, server)
        });
    });

});

// Bind HTTP server to a know port
httpServer.listen(11861);





