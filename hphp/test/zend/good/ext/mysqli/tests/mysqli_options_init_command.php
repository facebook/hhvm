<?php
	/* see mysqli.c for details */
	require_once("connect.inc");

	if (!($link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)))
		printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	/* TODO: test more options */
	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_options_init_command_table_1") ||
		!mysqli_query($link, sprintf("CREATE TABLE test_mysqli_options_init_command_table_1(id INT) ENGINE = %s\n", $engine)) ||
		!mysqli_query($link, "INSERT INTO test_mysqli_options_init_command_table_1(id) VALUES (1)"))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, "SELECT COUNT(id) AS _num_rows FROM test_mysqli_options_init_command_table_1"))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$row = mysqli_fetch_assoc($res);
	mysqli_free_result($res);

	if ($row['_num_rows'] != 1)
		printf("[003] Expecting 1 got %s\n", $row['_num_rows']);

	mysqli_close($link);

	$link = mysqli_init();
	if (true !== mysqli_options($link, MYSQLI_INIT_COMMAND, "INSERT INTO test_mysqli_options_init_command_table_1(id) VALUES (2)"))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket))
		printf("[005] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	if (!$res = mysqli_query($link, "SELECT COUNT(id) AS _num_rows FROM test_mysqli_options_init_command_table_1"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$row = mysqli_fetch_assoc($res);
	mysqli_free_result($res);

	if ($row['_num_rows'] != 2)
		printf("[007] Expecting 1 got %s\n", $row['_num_rows']);

	mysqli_close($link);

	$link = mysqli_init();
	if (true !== mysqli_options($link, MYSQLI_INIT_COMMAND, "INSERT INTO test_mysqli_options_init_command_table_1(i_do_no_exist) VALUES (2)"))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_close($link);

	$link = mysqli_init();
	if (true !== mysqli_options($link, MYSQLI_INIT_COMMAND, "INSERT INTO test_mysqli_options_init_command_table_1(i_do_no_exist) VALUES (2)"))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket))
		printf("[010] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_options_init_command_table_1'; require_once("clean_table.inc");
?>