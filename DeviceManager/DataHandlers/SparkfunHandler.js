/********
 * Requirements
 ********/
var moment = require('moment');
var http = require('http');
var BaseHandler = require('./BaseHandler');
var util = require('util');
var logins = require('../../logins');

/********
 * Variables and objects
 ********/

var host = "data.sparkfun.com";
var path = "/input/bGl0jYr1wgflYod32xpb";
var privateKey = logins.sparkfun.private_key;

function SparkfunHandler() {
  BaseHandler.apply( this, arguments );
}
util.inherits( SparkfunHandler, BaseHandler );

/********
 * Default settings
 ********/

SparkfunHandler.prototype.mapping = {
  'c' : 'temperature',
  'time' : 'time',
  'id': 'id',
};

SparkfunHandler.prototype.name = "Sparkfun";

SparkfunHandler.prototype.handleData = function( data, device ) {

  if( !data.time ) {
    data.time = moment().format();
  }
  if( !data.id ) {
    data.id = device.id;
  }
  var insertData = this.mapData( data, device );

  console.log( insertData );

  var body = JSON.stringify( insertData );
  var options = {
    host: host,
    path: path,
    method: 'POST',
    headers: {
        "Content-Type": "application/json",
        "Content-Length": Buffer.byteLength(body),
        "Phant-Private-Key": privateKey,
    }
  };
  var callback = function( response ) {
    var str = "";
    response.on( 'data', function( chunk ) {
      str += chunk;
    });
    response.on( 'end', function() {
      console.log( str );
    });
  }
  var request = http.request( options, callback );
  request.write( body );
  request.end();
}

module.exports = new SparkfunHandler();
