<?php

$a = NULL;
$b = NULL;
var_dump(stream_socket_client("", $a, $b));
var_dump($a, $b);
var_dump(stream_socket_client("[", $a, $b));
var_dump($a, $b);
var_dump(stream_socket_client("[ ", $a, $b));
var_dump($a, $b);
var_dump(stream_socket_client(".", $a, $b));
var_dump($a, $b);
var_dump(stream_socket_client(1, $a, $b));
var_dump($a, $b);
var_dump(stream_socket_client(array(), $a, $b));
var_dump($a, $b);

echo "Done\n";
?>