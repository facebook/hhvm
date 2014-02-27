<?php

require_once("connect.inc");
$mysqli = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

$result = $mysqli->query('select 1');

$result->close();
echo $result->num_rows;

$mysqli->close();
echo $result->num_rows;

echo "Done\n";
?>