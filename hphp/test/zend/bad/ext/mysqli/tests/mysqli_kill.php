<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_kill_table_1'; require('table.inc');

	// Zend will cast the NULL to 0
	if (!is_bool($tmp = mysqli_kill($link, null)))
		printf("[003] Expecting boolean/any, got %s/%s\n", gettype($tmp), $tmp);

	if (!$thread_id = mysqli_thread_id($link))
		printf("[004] Cannot determine thread id, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$tmp = mysqli_kill($link, $thread_id);
	if (!is_bool($tmp))
		printf("[005] Expecting boolean/any, got %s/%s\n", gettype($tmp), $tmp);

	if ($res = mysqli_query($link, "SELECT id FROM test_mysqli_kill_table_1 LIMIT 1"))
		pintf("[006] Expecting boolean/false, got %s/%s\n", gettype($res), $res);

	var_dump($error = mysqli_error($link));
	if (!is_string($error) || ('' === $error))
		printf("[007] Expecting string/any non empty, got %s/%s\n", gettype($error), $error);
	var_dump($res);
	var_dump($link);
	if ($IS_MYSQLND) {
		if ($link->info != 'Records: 6  Duplicates: 0  Warnings: 0') {
			printf("[008] mysqlnd used to be more verbose and used to support SELECT\n");
		}
		if ($link->stat != NULL) {
			printf("[009] NULL expected because of error.\n");
		}
	} else {
		if ($link->info != NULL) {
			printf("[008] Time for wonders - libmysql has started to support SELECT, change test\n");
		}
	}

	mysqli_close($link);

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[010] Cannot connect, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

	mysqli_kill($link, -1);
	if ((!$res = mysqli_query($link, "SELECT id FROM test_mysqli_kill_table_1 LIMIT 1")) ||
		(!$tmp = mysqli_fetch_assoc($res))) {
		printf("[011] Connection should not be gone, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	var_dump($tmp);
	mysqli_free_result($res);
	mysqli_close($link);

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[012] Cannot connect, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

	mysqli_change_user($link, "This might work if you accept anonymous users in your setup", "password", $db);      mysqli_kill($link, -1);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_kill_table_1'; require_once("clean_table.inc");
?>