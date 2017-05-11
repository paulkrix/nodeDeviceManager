
function BaseHandler() {};

BaseHandler.prototype.mapping = {};
BaseHandler.prototype.name = "Base";
BaseHandler.prototype.handleData = function( data, device ) {};

BaseHandler.prototype.init = function() {};

BaseHandler.prototype.getOption = function( option, device ) {
  if( device.dataHandlers[ this.name ].hasOwnProperty( option ) ) {
    return device.dataHandlers[ this.name ][ option ];
  }
  return this[ option ];
}

BaseHandler.prototype.mapData = function( data, device ) {
  var insertData = {};

  var mapping = this.getOption( 'mapping', device );
  for (var key in mapping) {
    if ( data.hasOwnProperty( key ) ) {
      insertData[ mapping[ key ] ] = data[ key ];
    }
  }

  return insertData;
}

module.exports = BaseHandler;
