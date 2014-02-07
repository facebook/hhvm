<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	}

	$tmp = mysqli_error($link);
	if (!is_string($tmp) || ('' !== $tmp))
		printf("[004] Expecting string/empty, got %s/%s. [%d] %s\n", gettype($tmp), $tmp, mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test')) {
		printf("[005] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_query($link, 'SELECT * FROM test');
	$tmp = mysqli_error($link);
	if (!is_string($tmp) || !preg_match("/Table '\w*\.test' doesn't exist/su", $tmp))
		printf("[006] Expecting string/[Table... doesn't exit], got %s/%s. [%d] %s\n", gettype($tmp), $tmp, mysqli_errno($link), mysqli_error($link));

	mysqli_close($link);

	var_dump(mysqli_error($link));

	print "done!";
?>