<?php
require 'connect.inc';
$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

mysqli_close($link);

$read = $error = $reject = array();
$read[] = $error[] = $reject[] = $link;

mysqli_poll($read, $error, $reject, 1);

echo "okey";
?>