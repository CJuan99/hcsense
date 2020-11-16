//import http package provided by Node
const http = require ('http');

/*const server = http.createServer((req, res) => {
  res.end ('This is my first respondse')
});

server.listen(3000);
*/

//import the constant app
const app = require ('./backend/app');

const port = 3000;


//to set the configuration for express
app.set ('port', port)


//pass the app to  create server
const server = http.createServer (app);

//http server object listen to the port ans execute a function
server.listen (port);
