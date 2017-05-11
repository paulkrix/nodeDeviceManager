/********
 * Requirements
 ********/
var MongoClient = require('mongodb').MongoClient;
var moment = require('moment');
var BaseHandler = require('./BaseHandler');
var util = require('util');

/********
 * Variables and objects
 ********/

var mongodbUrl = 'mongodb://localhost:27017/';

function MongoHandler() {
  BaseHandler.apply( this, arguments );
}
util.inherits( MongoHandler, BaseHandler );

/********
 * Default settings
 ********/
MongoHandler.prototype.mapping = {
  'c' : 'temperature',
  'time' : 'time',
  'id': 'id',
};

MongoHandler.prototype.collection = 'sensorData';
MongoHandler.prototype.database = 'brew';
MongoHandler.prototype.name = "Mongo";
MongoHandler.prototype.host = mongodbUrl;

MongoHandler.prototype.handleData = function( data, device ) {

  if( !data.time ) {
    data.time = moment().format();
  }
  if( !data.id ) {
    data.id = device.id;
  }

  var collection = this.getOption( 'collection', device );
  var database = this.getOption( 'database', device );
  var host = this.getOption( 'host', device );
  var insertData = this.mapData( data, device );

  MongoClient.connect( host + database, function( error, db ) {
    if( error !== null ) {
      db.close();
      return error;
    }

    db.collection( collection ).insertOne(
      insertData,
      function( error, result ) {
        db.close();
        if( error !== null ) {
          return error;
        }
        return 0;
      }
    );
  });
}

module.exports = new MongoHandler();
