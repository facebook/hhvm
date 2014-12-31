<?php
	/*
	NOTE: no datatype tests here! This is done by
	mysqli_stmt_bind_result.phpt already. Restrict
	this test case to the basics.
	*/
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_fetch_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	// stmt object status test
	if (NULL !== ($tmp = mysqli_stmt_fetch($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_fetch_table_1 ORDER BY id LIMIT 2"))
		printf("[005] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	// FIXME - different versions return different values ?!
	if ((NULL !== ($tmp = mysqli_stmt_fetch($stmt))) && (false !== $tmp))
		printf("[006] Expecting NULL or boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt))
		printf("[007] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_fetch($stmt)))
		printf("[008] NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_stmt_close($stmt);
	if (!$stmt = mysqli_stmt_init($link))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_fetch_table_1 ORDER BY id LIMIT 2"))
		printf("[010] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_execute($stmt))
		printf("[011] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = NULL;
	$label = NULL;
	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_stmt_fetch($stmt)))
		printf("[013] Expecting boolean/true, got %s/%s, [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_kill($link, mysqli_thread_id($link)))
		printf("[014] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (true !== ($tmp = mysqli_stmt_fetch($stmt)))
		printf("[015] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_fetch($stmt)))
		printf("[016] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	/* Check that the function alias exists. It's a deprecated function,
	but we have not announce the removal so far, therefore we need to check for it */
	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_stmt_fetch_table_1'; require_once("clean_table.inc");
?>