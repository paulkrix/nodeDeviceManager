//NPM packages

var http = require('http');
var device = require('./device');
var dataHandlers = require('./DataHandlers/dataHandlers');

// var CHECK_DEVICE_INTERVAL = 5*60*1000; //Check if device is responsive every 5 minutes
var CHECK_DEVICE_INTERVAL = 0; //Dont ping devices for status

function DeviceManager() {
  if( CHECK_DEVICE_INTERVAL !== 0 ) {
    this.setTimeout( '__checkDevices', CHECK_DEVICE_INTERVAL );
  }
}
DeviceManager.prototype._devices = [];

DeviceManager.prototype.setTimeout = function( callback, interval ) {
  var that = this;
  setTimeout( function() {
    that[callback]();
  }, interval );
}

DeviceManager.prototype.registerDevice = function( deviceOptions ) {
  var id = this._devices.length;
  console.log( deviceOptions );
  var newDevice = device( deviceOptions, id );
  this._devices[ id ] = newDevice;
  console.log( newDevice );
  //preload the data handlers
  dataHandlers.getDataHandlers( newDevice.dataHandlers );
  return id;
}

DeviceManager.prototype.getDevices = function() {
  return this._devices;
}

DeviceManager.prototype.getDevice = function() {
  return this._devices[ deviceId ];
}

DeviceManager.prototype.handleInput = function( deviceId, data ) {
  var _device = this._devices[ deviceId ];
  console.log( "logging for " + deviceId );
  dataHandlers.handleData( data, _device );
}

DeviceManager.prototype.handleControl = function( deviceId, data ) {
  var _device = this._devices[ deviceId ];
  console.log( "Sending control data to " + _device.ip + ":" + _device.port );
  var jsonData = JSON.stringify( data );

  var options = {
    host: _device.ip,
    port: _device.port,
    path: '/controls',
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
      'Content-Length': Buffer.byteLength( jsonData )
    }
  };
  var postRequest = http.request( options, function( response ) {
    response.on( 'data', function( chunk ) {
      console.log("Response: " + chunk );
    });
  });
  postRequest.write( jsonData );
  postRequest.end();
  return 0;
}

DeviceManager.prototype.__checkDevices = function() {
  console.log( "Checking " + this._devices.length + " devices");
  for( var i = 0; i < this._devices.length; i++ ) {
    this.__checkDevice( this._devices[i] );
  }
  this.setTimeout( '__checkDevices', CHECK_DEVICE_INTERVAL );
}

DeviceManager.prototype.__checkDevice = function( _device ) {
  var options = {
    host: _device.ip,
    port: _device.port,
    path: '/status'
  };
  http.request( options, function( response ) {
    var str = "";
    response.on( 'data', function( chunk ) {
      str += chunk;
    });
    response.on( 'end', function() {
      _device.status = str;
    });
  }).end();
}


/******
 * Private functions and variables
 ******/

module.exports = new DeviceManager();
