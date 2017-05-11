function DataHandlers() {};

DataHandlers.prototype.handleData = function( data, device ) {
  var dataHandlers = this.getDataHandlers( device.dataHandlers );
  if( dataHandlers.constructor !== Array ) {
    dataHandlers = [dataHandlers];
  }
  for( var i = 0; i < dataHandlers.length; i++) {
    dataHandlers[i].handleData( data, device );
  }
}

DataHandlers.prototype.getDataHandler = function( dataHandlerConfiguration, name ) {
  // Add it to the list if it doesn't already exist
  if( !this.hasOwnProperty( name ) ) {
    this[ name ] = require('./' + name + 'Handler' );
    this[ name ].init();
  }
  //return the handler
  return this[ name ];
}

DataHandlers.prototype.getDataHandlers = function( dataHandlerConfiguration ) {
  var dataHandlersArray = [];
  for( var name in dataHandlerConfiguration ) {
    dataHandlersArray.push( this.getDataHandler( dataHandlerConfiguration[ name ], name ) );
  }
  return dataHandlersArray;
}

module.exports = new DataHandlers();
