<?php


$fp = fsockopen("localhost", 8084, $errno, $errstr, 30);

if( !$fp ) {
	echo "$errstr ($errno)<br />\n";
} else {

  $data = '{"schema":{"database":"brew","collection":"temperature"},"ip":"127.0.0.1","port":8085}';

	$out = "POST /devices HTTP/1.1\n";
	$out .= "Host: localhost\n";
	$out .= "Content-Type: application/json; charset=utf-8\n";
	$out .= "Content-Length: ". strlen( $data ) . "\n";
	$out .= "Connection: Close\n\n";
  $out .= $data . "\n";
	echo $out . "\n";

	fwrite( $fp, $out );
	while( !feof( $fp ) ) {
		echo fgets( $fp, 128 );
	}
	fclose( $fp );
	echo "\n";
}

?>
