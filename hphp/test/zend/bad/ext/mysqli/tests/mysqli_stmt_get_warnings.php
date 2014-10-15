<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_get_warnings_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (NULL !== ($tmp = mysqli_stmt_get_warnings($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "SET sql_mode=''") || !mysqli_stmt_execute($stmt))
		printf("[005] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_prepare($stmt, "DROP TABLE IF EXISTS test") || !mysqli_stmt_execute($stmt))
		printf("[006] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (false !== ($tmp = mysqli_stmt_get_warnings($stmt)))
		printf("[007] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "CREATE TABLE test(id SMALLINT, label CHAR(1))") || !mysqli_stmt_execute($stmt))
		printf("[008] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (false !== ($tmp = mysqli_stmt_get_warnings($stmt)))
		printf("[009] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "INSERT INTO test(id, label) VALUES (100000, 'a'), (100001, 'b')") ||
		!mysqli_stmt_execute($stmt))
		printf("[010] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!is_object($warning = mysqli_stmt_get_warnings($stmt)))
		printf("[011] Expecting mysqli_warning object, got %s/%s\n", gettype($warning), $warning);

	if ('mysqli_warning' !== get_class($warning))
		printf("[012] Expecting object of type mysqli_warning got type '%s'", get_class($warning));

	if (!method_exists($warning, 'next'))
		printf("[013] Object mysqli_warning seems to lack method next()\n");

	$i = 0;
	do {

		if ('' == $warning->message)
			printf("[014 - %d] Message should not be empty\n", $i);

		if ('' == $warning->sqlstate)
			printf("[015 - %d] SQL State should not be empty\n", $i);

		if (0 == $warning->errno)
			printf("[016 - %d] Error number should not be zero\n", $i);

		$i++;

	} while ($warning->next());

	if (2 != $i)
		printf("[017] Expected 2 warnings, got %d warnings\n", $i);

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_get_warnings($stmt)))
		printf("[018] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);
	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_stmt_get_warnings_table_1'; require_once("clean_table.inc");
?>