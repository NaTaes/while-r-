const net = require('net')
const port = 8003;

server = net.createServer(function (client) {
    console.log('Client connection: ');
    console.log('   local = %s:%s', client.localAddress, client.localPort);
    //console.log('   remote = %s:%s', client.remoteAddress, client.remotePort);
    client.setTimeout(100);
    client.setEncoding('utf8');
    client.on('data', function (data) {
        console.log('Received data from client on port %d: %s', client.remotePort, data);
    });

    client.on('end', function () {
        console.log('Client disconnected');
        server.getConnections(function (err, count) {
            console.log('Remaining Connections: ' + count);
        });
    });

    client.on('error', function (err) {
        console.log('Socket Error: ', JSON.stringify(err));
    });

    client.on('timeout', function () {
        console.log('Socket Timed out');
    });
});

server.listen(port || 8083, function () {
    console.log('Server listening: ' + JSON.stringify(server.address())); // 서버 대기중
    server.on('close', function () {
        console.log('Server Terminated');
    });
    server.on('error', function (err) {
        console.log('Server Error: ', JSON.stringify(err));
    });
});