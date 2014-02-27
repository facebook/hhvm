<?php
	require_once("connect.inc");
	$link = mysqli_init();
	$link->options(MYSQLI_SET_CHARSET_NAME, "latin2");
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		die("can't connect");
	}
	var_dump($link->query("show variables like 'character_set_client'")->fetch_row());
	var_dump($link->query("show variables like 'character_set_connection'")->fetch_row());
	$link->change_user($user, $passwd, $db);
	var_dump($link->query("show variables like 'character_set_client'")->fetch_row());
	var_dump($link->query("show variables like 'character_set_connection'")->fetch_row());

	print "done!";
?>