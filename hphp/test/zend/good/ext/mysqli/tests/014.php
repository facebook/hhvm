<?php
	require_once("connect.inc");
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!mysqli_autocommit($link, TRUE))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_014_table_1"))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "CREATE TABLE test_014_table_1(a int, b varchar(10)) engine=InnoDB"))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "INSERT INTO test_014_table_1 VALUES (1, 'foobar')"))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_autocommit($link, FALSE))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "DELETE FROM test_014_table_1"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "INSERT INTO test_014_table_1 VALUES (2, 'egon')"))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_rollback($link))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$result = mysqli_query($link, "SELECT * FROM test_014_table_1"))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	printf("Num_of_rows=%d\n", mysqli_num_rows($result));
	if (!$row = mysqli_fetch_row($result))
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($result);

	var_dump($row);

	if (!mysqli_query($link, "DELETE FROM test_014_table_1"))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "INSERT INTO test_014_table_1 VALUES (2, 'egon')"))
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_commit($link))
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$result = mysqli_query($link, "SELECT * FROM test_014_table_1"))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_row($result))
		printf("[014] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($result);

	var_dump($row);

	mysqli_query($link, "DROP TABLE IF EXISTS test_014_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_014_table_1'; require_once("clean_table.inc");
?>