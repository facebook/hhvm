<?php

	require_once("connect.inc");

	$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	var_dump($mysqli->autocommit(false));
	$result = $mysqli->query("SELECT @@autocommit");
	var_dump($result->fetch_row());

	var_dump($mysqli->autocommit(true));
	$result = $mysqli->query("SELECT @@autocommit");
	var_dump($result->fetch_row());

?>