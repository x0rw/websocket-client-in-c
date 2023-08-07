const WebSocket = require('ws');

const ws = new WebSocket.Server({ 
          port: 8828});

console.log("Server Started");

ws.on('connection', (wss) => {

  wss.on('error', err => { console.log(err) })
  console.log("A new Client Connected")

  wss.send('Welcome to the server');


  wss.on('message', (message) => {
//     readline.question('What to s', name => {
//   readline.close();
// });
    console.log(`Server Received: ${message}`);


    if (message=="ping") {
      console.log(`Sending pong`);
      wss.send('pong');
    }else{
    wss.send('Got your Message: ' + message);
    console.log(`sending... Got your Message:${message}`);

    }

  });
wss.on('close', () => { console.log('close') })
});
