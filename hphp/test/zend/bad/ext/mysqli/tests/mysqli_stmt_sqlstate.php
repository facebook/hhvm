<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_sqlstate_table_1'; require('table.inc');
	if (!$stmt = mysqli_stmt_init($link))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (NULL !== ($tmp = mysqli_stmt_sqlstate($stmt)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "SELECT id FROM test_mysqli_stmt_sqlstate_table_1"))
		printf("[006] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if ('00000' !== ($tmp = mysqli_stmt_sqlstate($stmt)))
		printf("[007] Expecting string/00000, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (mysqli_stmt_prepare($stmt, "SELECT believe_me FROM i_dont_belive_that_this_table_exists"))
		printf("[008] Should fail! [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if ('' === ($tmp = mysqli_stmt_sqlstate($stmt)))
		printf("[009] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_sqlstate($stmt)))
		printf("[010] Expecting NULL, got %s/%s\n");

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_sqlstate_table_1'; require_once("clean_table.inc");
?>