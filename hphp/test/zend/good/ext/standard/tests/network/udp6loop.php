<?php
	/* Setup socket server */
	$server = stream_socket_server('udp://[::1]:31337', $errno, $errstr, STREAM_SERVER_BIND);
	if (!$server) {
		die('Unable to create AF_INET6 socket [server]');
	}

	/* Connect to it */
	$client = stream_socket_client('udp://[::1]:31337');
	if (!$client) {
		die('Unable to create AF_INET6 socket [client]');
	}

	fwrite($client, "ABCdef123\n");

	$data = fread($server, 10);
	var_dump($data);

	fclose($client);
	fclose($server);
?>