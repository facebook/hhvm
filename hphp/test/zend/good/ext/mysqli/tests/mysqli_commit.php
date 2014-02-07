<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[004] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	if (true !== ($tmp = mysqli_autocommit($link, false)))
		printf("[005] Cannot turn off autocommit, expecting true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test'))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'CREATE TABLE test(id INT) ENGINE = InnoDB'))
		printf("[007] Cannot create test table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'INSERT INTO test(id) VALUES (1)'))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$tmp = mysqli_commit($link);
	if ($tmp !== true)
		printf("[009] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_query($link, 'ROLLBACK'))
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS num FROM test'))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	$tmp = mysqli_fetch_assoc($res);
	if (1 != $tmp['num'])
		printf("[12] Expecting 1 row in table test, found %d rows\n", $tmp['num']);
	mysqli_free_result($res);

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test'))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_commit_table_1'; require_once("clean_table.inc");
?>