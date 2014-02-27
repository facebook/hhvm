<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$test_table_name = 'test_mysqli_query_iterators_table_1'; require('table.inc');

	echo "--- Testing default ---\n";
	if (!is_object($res = mysqli_query($link, "SELECT id FROM test_mysqli_query_iterators_table_1 ORDER BY id")))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	else {
		foreach ($res as $row) {
			var_dump($row);
		}
		echo "======\n";
		foreach ($res as $row) {
			var_dump($row);
		}
		mysqli_free_result($res);
		foreach ($res as $row) {
			var_dump($row);
		}
	}
	echo "--- Testing USE_RESULT ---\n";
	if (!is_object($res = mysqli_query($link, "SELECT id FROM test_mysqli_query_iterators_table_1 ORDER BY id", MYSQLI_USE_RESULT)))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	else {
		foreach ($res as $row) {
			var_dump($row);
		}
		echo "======\n";
		foreach ($res as $row) {
			var_dump($row);
		}
		mysqli_free_result($res);
	}

	echo "--- Testing STORE_RESULT ---\n";
	if (!is_object($res = mysqli_query($link, "SELECT id FROM test_mysqli_query_iterators_table_1 ORDER BY id", MYSQLI_STORE_RESULT)))
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	else {
		foreach ($res as $row) {
			var_dump($row);
		}
		echo "======\n";
		foreach ($res as $row) {
			var_dump($row);
		}
		mysqli_free_result($res);
	}

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_query_iterators_table_1'; require_once("clean_table.inc");
?>