<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_num_fields_table_1'; require('table.inc');

	function func_test_mysqli_num_fields($link, $query, $expected, $offset, $test_free = false) {

		if (!($res = mysqli_query($link, $query))) {
			printf("[%03d] [%d] %s\n", $offset, mysqli_errno($link), mysqli_error($link));
			return;
		}

		if ($expected !== ($tmp = mysqli_num_fields($res)))
			printf("[%03d] Expecting %s/%d, got %s/%d\n", $offset + 1,
				gettype($expected), $expected,
				gettype($tmp), $tmp);

		mysqli_free_result($res);

		if ($test_free && (NULL !== ($tmp = mysqli_num_fields($res))))
			printf("[%03d] Expecting NULL, got %s/%s\n", $offset + 2, gettype($tmp), $tmp);
	}

	func_test_mysqli_num_fields($link, "SELECT 1 AS a", 1, 5);
	func_test_mysqli_num_fields($link, "SELECT id, label FROM test_mysqli_num_fields_table_1", 2, 10);
	func_test_mysqli_num_fields($link, "SELECT 1 AS a, NULL AS b, 'foo' AS c", 3, 15);
	func_test_mysqli_num_fields($link, "SELECT id FROM test_mysqli_num_fields_table_1", 1, 20, true);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_num_fields_table_1'; require_once("clean_table.inc");
?>