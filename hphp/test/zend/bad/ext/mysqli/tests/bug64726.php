<?php
require 'connect.inc';
$db = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

$result = $db->query('SELECT 1', MYSQLI_USE_RESULT);
$db->close();
var_dump($result->fetch_object());
?>