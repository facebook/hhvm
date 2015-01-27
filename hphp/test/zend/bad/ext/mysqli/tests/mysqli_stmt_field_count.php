<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_field_count_table_1'; require('table.inc');

	$stmt = mysqli_stmt_init($link);
	if (!is_null($tmp = mysqli_stmt_field_count($stmt)))
		printf("[003] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (mysqli_stmt_prepare($stmt, ''))
		printf("[004] Prepare should fail for an empty statement\n");
	if (!is_null($tmp = mysqli_stmt_field_count($stmt)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, 'SELECT 1'))
		printf("[006] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	if (1 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[007] Expecting int/1, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, 'SELECT 1, 2'))
		printf("[008] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	if (2 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[009] Expecting int/2, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, 'SELECT id, label FROM test_mysqli_stmt_field_count_table_1'))
		printf("[010] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	if (2 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[011] Expecting int/2, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, 'SELECT label FROM test_mysqli_stmt_field_count_table_1') ||
		!mysqli_stmt_execute($stmt))
		printf("[012] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	if (1 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[013] Expecting int/1, got %s/%s\n", gettype($tmp), $tmp);

	$label = null;
	if (mysqli_stmt_bind_param($stmt, "s", $label))
		printf("[014] expected error - got ok\n");
	while (mysqli_stmt_fetch($stmt))
		if (1 !== ($tmp = mysqli_stmt_field_count($stmt)))
			printf("[015] Expecting int/1, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, 'INSERT INTO test_mysqli_stmt_field_count_table_1(id) VALUES (100)'))
		printf("[016] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	if (0 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[017] Expecting int/0, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "UPDATE test_mysqli_stmt_field_count_table_1 SET label = 'z' WHERE id = 1") ||
		!mysqli_stmt_execute($stmt))
		printf("[018] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (0 !== ($tmp = mysqli_stmt_field_count($stmt)))
		printf("[019] Expecting int/0, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_stmt_close($stmt);

	if (mysqli_stmt_prepare($stmt, 'SELECT id FROM test_mysqli_stmt_field_count_table_1'))
		printf("[020] Prepare should fail, statement has been closed\n");
	if (!is_null($tmp = mysqli_stmt_field_count($stmt)))
		printf("[011] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_stmt_field_count_table_1'; require_once("clean_table.inc");
?>