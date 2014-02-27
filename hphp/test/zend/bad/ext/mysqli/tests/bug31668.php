<?php
	require_once("connect.inc");

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);
	$mysql->multi_query('SELECT 1;SELECT 2');
	do {
		$res = $mysql->store_result();
		if ($mysql->errno == 0) {
			while ($arr = $res->fetch_assoc()) {
				var_dump($arr);
			}
			$res->free();
		}
	} while ($mysql->next_result());
	var_dump($mysql->error, __LINE__);
	$mysql->close();

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);
	$mysql->multi_query('SELECT 1;SELECT 2');
	do {
		$res = $mysql->store_result();
		if ($mysql->errno == 0) {
			while ($arr = $res->fetch_assoc()) {
				var_dump($arr);
			}
			$res->free();
		}
	} while ($mysql->next_result());
	var_dump($mysql->error, __LINE__);
?>