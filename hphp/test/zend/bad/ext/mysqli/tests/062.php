<?php
	require_once("connect.inc");

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	$mysql->real_query("SELECT 'foo' FROM test_062_table_1");

	$myresult = new mysqli_result($mysql);

	$row = $myresult->fetch_row();
	$myresult->close();
	$mysql->close();

	var_dump($row);
	print "done!";
?>