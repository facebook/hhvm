<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!$stmt = mysqli_prepare($link, "SELECT md5('bar'), database(), 'foo'"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_stmt_bind_result($stmt, $c0, $c1, $c2);
	mysqli_stmt_execute($stmt);

	mysqli_stmt_fetch($stmt);
	mysqli_stmt_close($stmt);

	$test = array($c0, $c1, $c2);
	if ($c1 !== $db) {
		echo "Different data\n";
	}

	var_dump($test);
	mysqli_close($link);
	print "done!";
?>