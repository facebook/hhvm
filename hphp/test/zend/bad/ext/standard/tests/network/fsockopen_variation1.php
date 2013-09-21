<?php
$port = rand(50000, 65535);

echo "Open a server socket\n";
$server = stream_socket_server('tcp://127.0.0.1:'.$port);

echo "\nCalling fsockopen() without a protocol in the hostname string:\n";
$hostname = '127.0.0.1';
$port = ''.$port;
$client = fsockopen($hostname, $port);
var_dump($client);
fclose($client);

echo "\nCalling fsockopen() with address and port in same string, without a protocol:\n";
$address = $hostname . ':' . $port;
$second_client = fsockopen($address);
var_dump($second_client);
fclose($second_client);

echo "Done";
?>