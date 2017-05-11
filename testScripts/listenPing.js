/*********
 * Required modules
 *********/
var express = require( 'express' );
var app = express();
var bodyParser = require( 'body-parser' );

/*********
 * Globals and configuration
 *********/

var externalIP = "0.0.0.0";
app.use( bodyParser.json() );
app.use( bodyParser.urlencoded( { extended: true} ) );

var httpPort = 8085;

/*********
 * Restful interface
 *********/

app.get('/status', function (req, res) {
  res.send('Running');
});

var server = app.listen( httpPort, function() {
  console.log(this._connectionKey);
  var host = server.address().address;
  var port = server.address().port;

  console.log("Example app listening at http://%s:%s", host, port);
});
