<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);
	$error = mysqli_error($link);
	var_dump($error);

	mysqli_select_db($link, $db);

	mysqli_query($link, "SELECT * FROM test_031_table_1");
	$error = mysqli_error($link);

	var_dump($error);

	mysqli_close($link);
	print "done!";
?>