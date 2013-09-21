<?php
    $socket = socket_create(AF_UNIX, SOCK_DGRAM, SOL_UDP); // cause warning
    $socket = socket_create(AF_UNIX, SOCK_DGRAM, 0);
    if (!$socket) {
        die('Unable to create AF_UNIX socket');
    }
    if (!socket_set_nonblock($socket)) {
        die('Unable to set nonblocking mode for socket');
    }
    var_dump(socket_recvfrom($socket, $buf, 12, 0, $from, $port)); //false (EAGAIN, no warning)
    $address = sprintf("/tmp/%s.sock", uniqid());
    if (!socket_bind($socket, $address)) {
        die("Unable to bind to $address");
    }

    $msg = "Ping!";
    $len = strlen($msg);
    $bytes_sent = socket_sendto($socket, $msg, $len, 0); // cause warning
    $bytes_sent = socket_sendto($socket, $msg, $len, 0, $address);
    if ($bytes_sent == -1) {
		@unlink($address);
        die('An error occurred while sending to the socket');
    } else if ($bytes_sent != $len) {
		@unlink($address);
        die($bytes_sent . ' bytes have been sent instead of the ' . $len . ' bytes expected');
    }

    $from = "";
    var_dump(socket_recvfrom($socket, $buf, 0, 0, $from)); // expect false
    $bytes_received = socket_recvfrom($socket, $buf, 12, 0, $from);
    if ($bytes_received == -1) {
		@unlink($address);
        die('An error occurred while receiving from the socket');
    } else if ($bytes_received != $len) {
		@unlink($address);
        die($bytes_received . ' bytes have been received instead of the ' . $len . ' bytes expected');
    }
    echo "Received $buf";

    socket_close($socket);
	@unlink($address);
?>