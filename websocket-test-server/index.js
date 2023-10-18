var app = require('http').createServer(handler)
var io = require('socket.io')(app);
var fs = require('fs');

console.log("listening on port 8080");
app.listen(8080);

function handler(req, res) {
        console.log("http req.");
        fs.readFile(__dirname + '/index.html',
                function (err, data) {
                        if (err) {
                                res.writeHead(500);
                                return res.end('Error loading index.html');
                        }

                        res.writeHead(200);
                        res.end(data);
                });
}

io.on('connection', function (socket) {
        console.log("new connection .. ", socket.id)
        socket.emit('news', { hello: 'world' });
        socket.on('my other event', function (data) {
                console.log(data);
        });
});

setInterval( () => {
        io.emit('schock', {strength: 50, id: 0xF737});
}, 5000);