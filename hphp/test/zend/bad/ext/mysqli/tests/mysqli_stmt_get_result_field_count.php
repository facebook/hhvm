<?php
	$test_table_name = 'test_mysqli_stmt_get_result_field_count_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_get_result_field_count_table_1 ORDER BY id ASC LIMIT 3"))
		printf("[002] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_execute($stmt))
		printf("[003] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!is_object($res = mysqli_stmt_get_result($stmt)) || 'mysqli_result' != get_class($res)) {
		printf("[004] Expecting object/mysqli_result got %s/%s, [%d] %s\n",
			gettype($res), $res, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	if (!is_object($res_meta = mysqli_stmt_result_metadata($stmt)) ||
		'mysqli_result' != get_class($res_meta)) {
		printf("[005] Expecting object/mysqli_result got %s/%s, [%d] %s\n",
			gettype($res), $res, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	printf("%s %s\n",
	$res_meta->field_count,
	$res->field_count);

	mysqli_stmt_close($stmt);
	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_get_result_field_count_table_1'; require_once("clean_table.inc");
?>