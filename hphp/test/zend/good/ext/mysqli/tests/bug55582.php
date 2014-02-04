<?php
	include "connect.inc";
	if (!($link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))) {
		printf("[001] Cannot connect to the server");
	}
	
	var_dump($link->real_query("SELECT 1"));
	$res = $link->use_result();
	var_dump(mysqli_num_rows($res));
	var_dump($res->fetch_assoc());
	var_dump(mysqli_num_rows($res));
	var_dump($res->fetch_assoc());
	var_dump(mysqli_num_rows($res));

	$link->close();
	echo "done\n";
?>