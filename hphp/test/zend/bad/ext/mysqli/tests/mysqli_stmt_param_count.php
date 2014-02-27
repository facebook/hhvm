<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_param_count_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (NULL !== ($tmp = mysqli_stmt_param_count($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	function func_test_mysqli_stmt_param_count_table_1_mysqli_stmt_param_count($stmt, $query, $expected, $offset) {

		if (!mysqli_stmt_prepare($stmt, $query)) {
			printf("[%03d] [%d] %s\n", $offset, mysqli_stmt_errno($stmt), mysqli_error($stmt));
			return false;
		}

		if ($expected !== ($tmp = mysqli_stmt_param_count($stmt)))
			printf("[%03d] Expecting %s/%d, got %s/%d\n", $offset + 3,
				gettype($expected), $expected,
				gettype($tmp), $tmp);
		return true;
	}

	func_test_mysqli_stmt_param_count_table_1_mysqli_stmt_param_count($stmt, "SELECT 1 AS a", 0, 10);
	func_test_mysqli_stmt_param_count_table_1_mysqli_stmt_param_count($stmt, "INSERT INTO test_mysqli_stmt_param_count_table_1(id) VALUES (?)", 1, 20);
	func_test_mysqli_stmt_param_count_table_1_mysqli_stmt_param_count($stmt, "INSERT INTO test_mysqli_stmt_param_count_table_1(id, label) VALUES (?, ?)", 2, 30);
	func_test_mysqli_stmt_param_count_table_1_mysqli_stmt_param_count($stmt, "INSERT INTO test_mysqli_stmt_param_count_table_1(id, label) VALUES (?, '?')", 1, 40);

	mysqli_stmt_close($stmt);

	if (NULL !== ($tmp = mysqli_stmt_param_count($stmt)))
		printf("[40] Expecting NULL, got %s/%s\n");

	mysqli_close($link);

	/* Check that the function alias exists. It's a deprecated function,
	but we have not announce the removal so far, therefore we need to check for it */
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_param_count_table_1'; require_once("clean_table.inc");
?>