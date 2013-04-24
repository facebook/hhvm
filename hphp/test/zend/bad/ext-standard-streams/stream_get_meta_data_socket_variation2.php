<?php

/* Setup socket server */
$server = stream_socket_server('tcp://127.0.0.1:31337');

/* Connect to it */
$client = fsockopen('tcp://127.0.0.1:31337');
if (!$client) {
	die("Unable to create socket");
}

/* Accept that connection */
$socket = stream_socket_accept($server);

var_dump(stream_get_meta_data($client));

echo "\n\nSet a timeout on the client and attempt a read:\n";
socket_set_timeout($client, 0, 1000);
fread($client, 1);
var_dump(stream_get_meta_data($client));

echo "\n\nWrite some data from the server:\n";
fwrite($socket, "12345");
var_dump(stream_get_meta_data($client));

echo "\n\nRead some data from the client:\n";
fread($client, 5);
var_dump(stream_get_meta_data($client));

fclose($client);
fclose($socket);
fclose($server);

?>