<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (true !== ($tmp = mysqli_report(-1)))
		printf("[002] Expecting boolean/true even for invalid flags, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_report(MYSQLI_REPORT_ERROR)))
		printf("[003] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_report(MYSQLI_REPORT_STRICT)))
		printf("[004] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_report(MYSQLI_REPORT_INDEX)))
		printf("[005] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_report(MYSQLI_REPORT_ALL)))
		printf("[007] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_report(MYSQLI_REPORT_OFF)))
		printf("[008] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	$test_table_name = 'test_mysqli_report_wo_ps_table_1'; require('table.inc');

	/*
	Internal macro MYSQL_REPORT_ERROR
	*/
	mysqli_report(MYSQLI_REPORT_ERROR);

	mysqli_multi_query($link, "BAR; FOO;");
	mysqli_query($link, "FOO");
	mysqli_change_user($link, "0123456789-10-456789-20-456789-30-456789-40-456789-50-456789-60-456789-70-456789-80-456789-90-456789", "password", $db);
	mysqli_kill($link, -1);

	// mysqli_ping() cannot be tested, because one would need to cause an error inside the C function to test it
	mysqli_real_query($link, "FOO");
	if (@mysqli_select_db($link, "Oh lord, let this be an unknown database name"))
		printf("[009] select_db should have failed\n");
	// mysqli_store_result() and mysqli_use_result() cannot be tested, because one would need to cause an error inside the C function to test it


	// Check that none of the above would have caused any error messages if MYSQL_REPORT_ERROR would
	// not have been set. If that would be the case, the test would be broken.
	mysqli_report(MYSQLI_REPORT_OFF);

	mysqli_multi_query($link, "BAR; FOO;");
	mysqli_query($link, "FOO");
	mysqli_change_user($link, "This might work if you accept anonymous users in your setup", "password", $db);
	mysqli_kill($link, -1);
	mysqli_real_query($link, "FOO");
	mysqli_select_db($link, "Oh lord, let this be an unknown database name");

	mysqli_report(MYSQLI_REPORT_OFF);
	mysqli_report(MYSQLI_REPORT_STRICT);

	try {

		if ($link = my_mysqli_connect($host, $user . 'unknown_really', $passwd . 'non_empty', $db, $port, $socket))
			printf("[010] Can connect to the server using host=%s, user=%s, passwd=***non_empty, dbname=%s, port=%s, socket=%s\n",
				$host, $user . 'unknown_really', $db, $port, $socket);
		mysqli_close($link);

	} catch (mysqli_sql_exception $e) {
		printf("[011] %s\n", $e->getMessage());
	}

	try {
		if (!$link = mysqli_init())
			printf("[012] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

		if ($link = my_mysqli_real_connect($link, $host, $user . 'unknown_really', $passwd . 'non_empty', $db, $port, $socket))
			printf("[013] Can connect to the server using host=%s, user=%s, passwd=***non_empty, dbname=%s, port=%s, socket=%s\n",
				$host, $user . 'unknown_really', $db, $port, $socket);
		mysqli_close($link);
	} catch (mysqli_sql_exception $e) {
		printf("[014] %s\n", $e->getMessage());
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_report_wo_ps_table_1'; require_once("clean_table.inc");
?>