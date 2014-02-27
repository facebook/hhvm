<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_execute_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	// stmt object status test
	if (NULL !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (mysqli_stmt_prepare($stmt, "SELECT i_do_not_exist_believe_me FROM test_mysqli_stmt_execute_table_1 ORDER BY id"))
		printf("[005] Statement should have failed!\n");

	// stmt object status test
	if (NULL !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[006] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "SELECT id FROM test_mysqli_stmt_execute_table_1 ORDER BY id LIMIT 1"))
		printf("[007] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[008] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_prepare($stmt, "INSERT INTO test_mysqli_stmt_execute_table_1(id, label) VALUES (?, ?)"))
		printf("[009] [%d] %s\n", mysqli_stmt_execute($stmt), mysqli_stmt_execute($stmt));

	// no input variables bound
	if (false !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[010] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	$id = 100;
	$label = "z";
	if (!mysqli_stmt_bind_param($stmt, "is", $id, $label))
		printf("[011] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[012] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	// calling reset between executions
	mysqli_stmt_close($stmt);
	if (!$stmt = mysqli_stmt_init($link))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "SELECT id FROM test_mysqli_stmt_execute_table_1 ORDER BY id LIMIT ?"))
		printf("[014] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$limit = 1;
	if (!mysqli_stmt_bind_param($stmt, "i", $limit))
		printf("[015] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[016] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = null;
	if (!mysqli_stmt_bind_result($stmt, $id) || !mysqli_stmt_fetch($stmt))
		printf("[017] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if ($id !== 1)
		printf("[018] Expecting int/1 got %s/%s\n", gettype($id), $id);

	if (true !== ($tmp = mysqli_stmt_reset($stmt)))
		printf("[019] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[020] Expecting boolean/true after reset to prepare status, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = null;
	if (!mysqli_stmt_fetch($stmt))
		printf("[021] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if ($id !== 1)
		printf("[022] Expecting int/1 got %s/%s\n", gettype($id), $id);

	mysqli_stmt_close($stmt);
	if (!$stmt = mysqli_stmt_init($link))
		printf("[023] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "SELECT id FROM test_mysqli_stmt_execute_table_1 ORDER BY id LIMIT 1"))
		printf("[024] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[025] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_reset($stmt)))
		printf("[026] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump(mysqli_stmt_execute($stmt));
	var_dump(mysqli_stmt_fetch($stmt));

	mysqli_kill($link, mysqli_thread_id($link));

	if (false !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[027] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_execute($stmt)))
		printf("[028] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_execute_table_1'; require_once("clean_table.inc");
?>