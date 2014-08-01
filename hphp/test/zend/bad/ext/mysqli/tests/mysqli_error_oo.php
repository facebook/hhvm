<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$mysqli = new mysqli();
	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	$tmp = $mysqli->error;
	if (!is_string($tmp) || ('' !== $tmp))
		printf("[003] Expecting string/empty, got %s/%s. [%d] %s\n", gettype($tmp), $tmp, $mysqli->errno, $mysqli->error);

	if (!$mysqli->query('DROP TABLE IF EXISTS test_mysqli_error_oo_table_1')) {
		printf("[004] Failed to drop old test table: [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	$mysqli->query('SELECT * FROM test_mysqli_error_oo_table_1');
	$tmp = $mysqli->error;
	if (!is_string($tmp) || !preg_match("/Table '\w*\.test_mysqli_error_oo_table_1' doesn't exist/su", $tmp))
		printf("[006] Expecting string/[Table... doesn't exit], got %s/%s. [%d] %s\n", gettype($tmp), $tmp, $mysqli->errno, $mysqli->error);

	$mysqli->close();

	var_dump($mysqli->error);

	print "done!";
?>