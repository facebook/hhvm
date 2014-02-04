<?php
	include "connect.inc";
	$db1 = new mysqli();

	// These calls fail
	$db1->options(MYSQLI_OPT_CONNECT_TIMEOUT, 3);
	my_mysqli_real_connect($db1, $host, $user, $passwd, $db, $port, $socket);
	if(mysqli_connect_error()) {
		echo "error 1\n";
	} else {
		echo "ok 1\n";
	}

	$db2 = mysqli_init();

	$db2->options(MYSQLI_OPT_CONNECT_TIMEOUT, 3);
	my_mysqli_real_connect($db2, $host, $user, $passwd, $db, $port, $socket);
	if(mysqli_connect_error()) {
		echo "error 2\n";
	} else {
		echo "ok 2\n";
	}
	echo "done\n";
?>