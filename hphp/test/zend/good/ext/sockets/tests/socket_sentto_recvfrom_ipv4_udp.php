<?php
    $socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);
    if (!$socket) {
        die('Unable to create AF_INET socket');
    }
    if (!socket_set_nonblock($socket)) {
        die('Unable to set nonblocking mode for socket');
    }

    $address = '127.0.0.1';
    try { socket_sendto($socket, '', 1, 0, $address); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // cause warning
    if (!socket_bind($socket, $address, 1223)) {
        die("Unable to bind to $address:1223");
    }

	var_dump(socket_recvfrom($socket, &$buf, 12, 0, &$from, &$port)); //false (EAGAIN - no warning)

    $msg = "Ping!";
    $len = strlen($msg);
    $bytes_sent = socket_sendto($socket, $msg, $len, 0, $address, 1223);
    if ($bytes_sent == -1) {
        die('An error occurred while sending to the socket');
    } else if ($bytes_sent != $len) {
        die($bytes_sent . ' bytes have been sent instead of the ' . $len . ' bytes expected');
    }

    $from = "";
    $port = 0;
    try { socket_recvfrom($socket, &$buf, 12, 0); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // cause warning
    try { socket_recvfrom($socket, &$buf, 12, 0, &$from); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; } // cause warning
    $bytes_received = socket_recvfrom($socket, &$buf, 12, 0, &$from, &$port);
    if ($bytes_received == -1) {
        die('An error occurred while receiving from the socket');
    } else if ($bytes_received != $len) {
        die($bytes_received . ' bytes have been received instead of the ' . $len . ' bytes expected');
    }
    echo "Received $buf from remote address $from and remote port $port" . PHP_EOL;

    socket_close($socket);
