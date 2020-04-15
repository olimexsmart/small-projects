const net = require("net")
const fs = require("fs")
const http = require("http")

let httpServer = http.createServer((request, response) => {
    let name = 'default'
    let size = 0

    let body = [];
    request.on('error', (err) => {
        console.error(err);
    }).on('data', (chunk) => {
        body.push(chunk);
    }).on('end', () => {
        data = JSON.parse(Buffer.concat(body).toString())
        name = data.filename
        size = data.size

        console.log(name + size)
    });

    let server = net.createServer()

    server.on("connection", socket => {
        let fileStream = fs.createWriteStream(name);

        socket.pipe(fileStream);

        socket.on("end", () => {
            server.close(() => { console.log("\nTransfer is done!") });
        })
    })

    server.on('error', error => {
        console.log(error)
    })

    server.listen({
        host: 'localhost',
        port: 0
    }, () => {
        port = server.address().port.toString()
        console.log(port)
        response.writeHead(200);
        response.end(port);
    });


});

httpServer.listen({
    host: 'localhost',
    port: 11861
});





