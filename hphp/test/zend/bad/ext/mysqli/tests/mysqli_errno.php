<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
}

	var_dump(mysqli_errno($link));

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test_mysqli_errno_table_1')) {
		printf("[004] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_query($link, 'SELECT * FROM test_mysqli_errno_table_1');
	var_dump(mysqli_errno($link));

	@mysqli_query($link, 'No SQL');
	if (($tmp = mysqli_errno($link)) == 0)
		printf("[005] Expecting int/any non zero got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	var_dump(mysqli_errno($link));

	print "done!";
?>