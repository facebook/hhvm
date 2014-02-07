<?php
	/*
	NOTE: no datatype tests here! This is done by
	mysqli_stmt_bind_result.phpt already. Restrict
	this test case to the basics.
	*/
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_init_table_1'; require('table.inc');

	if (!is_object($stmt = mysqli_stmt_init($link)))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_object($stmt2 = @mysqli_stmt_init($link)))
		printf("[003a] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_init($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_stmt_init($link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_init_table_1'; require_once("clean_table.inc");
?>