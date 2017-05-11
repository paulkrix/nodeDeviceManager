<?php

$server_ip = '255.255.255.255';
$server_port = 33333;
$local_port = 33334;
$message = 'device_discovery';

if ($socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP)) {
    socket_set_option($socket, SOL_SOCKET, SO_BROADCAST, 1);
    socket_sendto($socket, $message, strlen($message), 0, $server_ip, $server_port);

    //Wait for response
    $waitForResponse = true;
    while( $waitForResponse )
    {
        echo "\n Waiting for data ... \n";

        //Receive some data
        $r = socket_recvfrom($socket, $buf, 512, 0, $remote_ip, $remote_port);
        echo "$remote_ip : $remote_port -- " . $buf . "\n";
        $waitForResponse = false;
            //Send back the data to the client
        //socket_sendto($sock, "OK " . $buf , 100 , 0 , $remote_ip , $remote_port);

    }

}
socket_close($socket);
//
//
// if ($socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP)) {
//
//     // Bind the source address
//       if( !socket_bind($socket, "0.0.0.0" , $local_port) )
//       {
//           $errorcode = socket_last_error();
//           $errormsg = socket_strerror($errorcode);
//
//           die("Could not bind socket : [$errorcode] $errormsg \n");
//       }
//       echo "Socket bind OK \n";
//
//       //Do some communication, this loop can handle multiple clients
//       while(1)
//       {
//           echo "\n Waiting for data ... \n";
//
//           //Receive some data
//           $r = socket_recvfrom($socket, $buf, 512, 0, $remote_ip, $remote_port);
//           echo "$remote_ip : $remote_port -- " . $buf;
//
//               //Send back the data to the client
//           //socket_sendto($sock, "OK " . $buf , 100 , 0 , $remote_ip , $remote_port);
//
//       }
//
//       socket_close($socket);
//
// }

?>
