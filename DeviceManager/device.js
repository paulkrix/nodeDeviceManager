function Device( options, id ) {
  if( options.type ) {
    this.type = options.type;
  }
  if( options.mode ) {
    this.mode = options.mode;
  }
  if( options.ip ) {
    this.ip = options.ip;
  }
  if( options.port ) {
    this.port = options.port;
  }
  if( options.logger ) {
    this.logger = options.logger;
  }

  this.id = id;
}

Device.prototype.id = null;
Device.prototype.type = "sensor";
Device.prototype.mode = "active"; //active | passive
Device.prototype.ip = null;
Device.prototype.port = "80";
Device.prototype.status = "registered";

// Device.prototype.dataHandlers = {
//   "Sparkfun" : {},
// }

Device.prototype.dataHandlers = {
  // Mongo : {
  //   options : {},
  //   mapping: {
  //     c : 'temperature',
  //     time : 'time',
  //     id: 'id',
  //   },
  // },
  Blynk : {
    mapping : {
      c: 2
    },
  }
}


function device (options, id ) {
  if (options === undefined) {
    return new Device({}, id)
  }

  if (typeof options === 'object' && options !== null) {
    return new Device(options, id)
  }

  throw new TypeError('Expected object for argument options')
}

module.exports = device;
