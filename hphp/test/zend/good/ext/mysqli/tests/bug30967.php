<?php
	require_once("connect.inc");

	class mysql1 extends mysqli {
	}

	class mysql2 extends mysql1 {
	}

	$mysql = new mysql2($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("THIS DOES NOT WORK");
	printf("%d\n", $mysql->errno);

	$mysql->close();
?>