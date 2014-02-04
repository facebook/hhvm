<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$stmt = mysqli_prepare($link, "SHOW VARIABLES LIKE 'port'");
	mysqli_stmt_execute($stmt);

	mysqli_stmt_bind_result($stmt, $c1, $c2);
	mysqli_stmt_fetch($stmt);
	mysqli_stmt_close($stmt);
	if ((version_compare(PHP_VERSION, '5.9.9', '>') == 1) && mysqli_get_server_version($link) < 50000) {
		/* variables are binary */
		settype($c1, "unicode");
		settype($c2, "unicode");
	}
	$test = array ($c1,$c2);

	var_dump($test);

	mysqli_close($link);
	print "done!";
?>