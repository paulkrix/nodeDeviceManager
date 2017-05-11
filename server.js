/*********
 * Required modules
 *********/
var express = require( 'express' );
var app = express();
var bodyParser = require( 'body-parser' );
var fs = require( 'fs' );
var assert = require('assert');
var dgram = require('dgram');
var DeviceManager = require('./DeviceManager/deviceManager');

/*********
 * Globals and configuration
 *********/

var UDPSocket = dgram.createSocket('udp4');
var externalIP = "0.0.0.0";
var UDPPort = 4211;

app.use( bodyParser.json() );
app.use( bodyParser.urlencoded( { extended: true} ) );

var httpPort = 8084;

/*********
 * UDP listener
 *********/
 /*
  * Makes the server discoverable to devices that don't know it's IP or port
  */

UDPSocket.on('listening', function() {
  var address = UDPSocket.address();
  console.log('UDP Server listening on ' + address.address + ":" + address.port);
});

UDPSocket.on('message', function (message, remote) {
    console.log(remote.address + ':' + remote.port +' - ' + message);
    var message = new Buffer( "" + httpPort );
    UDPSocket.send(message, 0, message.length, remote.port, remote.address, function(err, bytes) {
      if (err) throw err;
      console.log('UDP message sent to ' + remote.address +':'+ remote.port);
    });
});

UDPSocket.bind( UDPPort, externalIP );

/*********
 * Restful interface
 *********/

app.get('/devices', function( request, response ) {
  response.status( 200 ).json( DeviceManager.getDevices() );
});

app.post('/devices', function( request, response ) {
  var newDeviceId = DeviceManager.registerDevice( request.body );
  response.status(201).json( { 'dataUrl':'/devices/'+newDeviceId+'/input', 'id':newDeviceId } );
});

app.get('/devices/:deviceId', function( request, response ) {
  var deviceId =  request.params.deviceId;
  response.status( 200 ).json( DeviceManager.getDevice( deviceId ) );
});

app.post('/devices/:deviceId/input', function( request, response ) {
  var deviceId =  request.params.deviceId;
  var result = DeviceManager.handleInput( deviceId, request.body );

  if( result === 0 ) {
    response.sendStatus( 201 );
  } else {
    response.status( 500 ).send( result );
  }
});

app.post('/devices/:deviceId/controls', function( request, response ) {
  var deviceId =  request.params.deviceId;
  var result = DeviceManager.handleControl( deviceId, request.body );

  if( result === 0 ) {
    response.sendStatus( 200 );
  } else {
    response.status( 500 ).send( result );
  }

});

app.get('/', function (req, res) {
  res.send('Hello World!')
});

// A handy endpoint for testing json requests. It just echoes the json payload
// back
app.post('/echo', function( request, response ) {
  console.log( request.body );
  response.json( request.body );
});

var server = app.listen( httpPort, function() {
  console.log(this._connectionKey);

  var host = server.address().address;
  var port = server.address().port;

  console.log("Example app listening at http://%s:%s", host, port);

});
