<?php
	require_once("connect.inc");

	// Note: No SQL tests here! We can expand one of the *fetch()
	// tests to a generic SQL test, if we ever need that.
	// We would duplicate the SQL test cases if we have it here and in one of the
	// fetch tests, because the fetch tests would have to call prepare/execute etc.
	// anyway.

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_prepare_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	if (false !== ($tmp = mysqli_stmt_prepare($stmt, '')))
		printf("[005] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (true !== ($tmp = mysqli_stmt_prepare($stmt, 'SELECT id FROM test_mysqli_stmt_prepare_table_1')))
		printf("[006] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_prepare($stmt, "SELECT id FROM test_mysqli_stmt_prepare_table_1")))
		printf("[007] Expecting NULL, got %s/%s\n");

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_prepare_table_1'; require_once("clean_table.inc");
?>