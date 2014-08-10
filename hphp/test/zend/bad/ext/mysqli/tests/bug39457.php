<?php
	require_once("connect.inc");

	$mysql = mysqli_init();
	$mysql->connect($host, $user, $passwd, $db, $port, $socket);

	$mysql->connect($host, $user, $passwd, $db, $port, $socket);

	$mysql->close();
	echo "OK\n";
?>