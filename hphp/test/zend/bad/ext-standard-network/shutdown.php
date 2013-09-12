<?php
$port = rand(50000, 65535);
	/* Setup socket server */
	$server = stream_socket_server('tcp://127.0.0.1:'.$port);
	if (!$server) {
		die('Unable to create AF_INET socket [server]');
	}

	/* Connect and send request 1 */
	$client1 = stream_socket_client('tcp://127.0.0.1:'.$port);
	if (!$client1) {
		die('Unable to create AF_INET socket [client]');
	}
	@fwrite($client1, "Client 1\n");
	stream_socket_shutdown($client1, STREAM_SHUT_WR);
	@fwrite($client1, "Error 1\n");

	/* Connect and send request 2 */
	$client2 = stream_socket_client('tcp://127.0.0.1:'.$port);
	if (!$client2) {
		die('Unable to create AF_INET socket [client]');
	}
	@fwrite($client2, "Client 2\n");
	stream_socket_shutdown($client2, STREAM_SHUT_WR);
	@fwrite($client2, "Error 2\n");

	/* Accept connection 1 */
	$socket = stream_socket_accept($server);
	if (!$socket) {
		die('Unable to accept connection');
	}
	@fwrite($socket, fgets($socket));
	@fwrite($socket, fgets($socket));
	fclose($socket);

	/* Read Response 1 */
	echo fgets($client1);
	echo fgets($client1);

	/* Accept connection 2 */
	$socket = stream_socket_accept($server);
	if (!$socket) {
		die('Unable to accept connection');
	}
	@fwrite($socket, fgets($socket));
	@fwrite($socket, fgets($socket));
	fclose($socket);

	/* Read Response 2 */
	echo fgets($client2);
	echo fgets($client2);

	fclose($client1);
	fclose($client2);
	fclose($server);
?>