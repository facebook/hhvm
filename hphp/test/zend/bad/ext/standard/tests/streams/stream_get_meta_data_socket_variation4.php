<?php

/* Setup socket server */
$server = stream_socket_server('tcp://127.0.0.1:31334');

/* Connect to it */
$client = fsockopen('tcp://127.0.0.1:31334');
if (!$client) {
	die("Unable to create socket");
}

/* Accept that connection */
$socket = stream_socket_accept($server);

echo "Write some data:\n";
fwrite($socket, "abcdefg\n1234567\nxyzxyz\n");
var_dump(stream_get_meta_data($client));

echo "\n\nRead a line from the client:\n";
fgets($client);
var_dump(stream_get_meta_data($client));

echo "\n\nClose the server side socket and read the remaining data from the client:\n";
fclose($socket);
fclose($server);
while(!feof($client)) {
	fread($client, 1);
}
var_dump(stream_get_meta_data($client));

fclose($client);

?>