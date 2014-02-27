<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$mysqli = new mysqli();
	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	var_dump($mysqli->errno);

	if (!$mysqli->query('DROP TABLE IF EXISTS test_mysqli_errno_oo_table_1')) {
		printf("[003] Failed to drop old test table: [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	$mysqli->query('SELECT * FROM test_mysqli_errno_oo_table_1');
	var_dump($mysqli->errno);

	@$mysqli->query('No SQL');
	if (($tmp = $mysqli->errno) === 0)
		printf("[004] Expecting int/any non zero got %s/%s\n", gettype($tmp), $tmp);

	$mysqli->close();

	var_dump($mysqli->errno);

	print "done!";
?>