<?php

function test($stream, $sock) {
	if ($stream !== null) {
		echo "stream_set_blocking ";
		print_r(stream_set_blocking($stream, 0));
		echo "\n";
	}
	if ($sock !== null) {
		echo "socket_set_block ";
		print_r(socket_set_block($sock));
		echo "\n";
		echo "socket_get_option ";
		print_r(socket_get_option($sock, SOL_SOCKET, SO_TYPE));
		echo "\n";
	}
	echo "\n";
}

echo "normal\n";
$stream0 = stream_socket_server("udp://0.0.0.0:58380", $errno, $errstr, STREAM_SERVER_BIND);
$sock0 = socket_import_stream($stream0);
test($stream0, $sock0);

echo "\nunset stream\n";
$stream1 = stream_socket_server("udp://0.0.0.0:58381", $errno, $errstr, STREAM_SERVER_BIND);
$sock1 = socket_import_stream($stream1);
unset($stream1);
test(null, $sock1);

echo "\nunset socket\n";
$stream2 = stream_socket_server("udp://0.0.0.0:58382", $errno, $errstr, STREAM_SERVER_BIND);
$sock2 = socket_import_stream($stream2);
unset($sock2);
test($stream2, null);

echo "\nclose stream\n";
$stream3 = stream_socket_server("udp://0.0.0.0:58383", $errno, $errstr, STREAM_SERVER_BIND);
$sock3 = socket_import_stream($stream3);
fclose($stream3);
test($stream3, $sock3);

echo "\nclose socket\n";
$stream4 = stream_socket_server("udp://0.0.0.0:58384", $errno, $errstr, STREAM_SERVER_BIND);
$sock4 = socket_import_stream($stream4);
socket_close($sock4);
test($stream4, $sock4);

echo "Done.\n";