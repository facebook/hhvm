<?php
	require_once("connect.inc");

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("DROP TABLE IF EXISTS not_exists");

	var_dump($mysql->warning_count);

	$w = $mysql->get_warnings();

	var_dump($w->errno);
	var_dump($w->message);
	var_dump($w->sqlstate);

	$mysql->close();
	echo "done!"
?>