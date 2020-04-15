const net = require("net"), fs = require("fs");



let server = net.createServer()

server.on("connection", socket => {
    let fileStream = fs.createWriteStream("received.png");

    

    socket.pipe(fileStream);

    socket.on("end", () => {
        server.close(() => { console.log("\nTransfer is done!") });
    })
})


server.listen({
    host: 'localhost',
    port: 11861
});