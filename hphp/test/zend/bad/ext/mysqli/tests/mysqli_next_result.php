<?php
	require_once("connect.inc");

	$strict_on = false;
	if (defined('E_STRICT')) {
		error_reporting(((int)ini_get('error_reporting')) | E_STRICT );
		$strict_on = true;
	}

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_next_result_table_1'; require('table.inc');

	if ($strict_on)
		ob_start();

	if (false !== ($tmp = mysqli_next_result($link)))
		printf("[003] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[003a] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
		print trim($tmp) . "\n";
		ob_start();
	}

	$res = mysqli_query($link, "SELECT 1 AS res");
	if (false !== ($tmp = mysqli_next_result($link)))
		printf("[004] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[004a] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
		print trim($tmp) . "\n";
	}

	mysqli_free_result($res);

	function func_test_mysqli_next_result($link, $query, $offset, $num_results, $strict_on) {

		if (!mysqli_multi_query($link, $query))
			printf("[%03d] [%d] %s\n", $offset, mysqli_errno($link), mysqli_error($link));

		$i = 0;
		if ($strict_on)
			ob_start();

		do {
			if ($res = mysqli_store_result($link)) {
				mysqli_free_result($res);
				$i++;
			}
		} while (true === mysqli_next_result($link));

		if ($strict_on) {
			$tmp = ob_get_contents();
			ob_end_clean();
			if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
				printf("[%03d] Strict Standards warning missing\n", $offset + 1);
			} else {
				$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
			}
			print trim($tmp) . "\n";
		}

		if ($i !== $num_results) {
			printf("[%03d] Expecting %d result(s), got %d result(s)\n", $offset + 2, $num_results, $i);
		}

		if (mysqli_more_results($link))
			printf("[%03d] mysqli_more_results() indicates more results than expected\n", $offset + 3);

		if (!($res = mysqli_query($link, "SELECT 1 AS b"))) {
			printf("[%03d] [%d] %s\n", $offset + 4, mysqli_errno($link), mysqli_error($link));
		} else {
			mysqli_free_result($res);
		}

	}

	func_test_mysqli_next_result($link, "SELECT 1 AS a; SELECT 1 AS a, 2 AS b; SELECT id FROM test_mysqli_next_result_table_1 ORDER BY id LIMIT 3", 5, 3, $strict_on);
	func_test_mysqli_next_result($link, "SELECT 1 AS a; INSERT INTO test_mysqli_next_result_table_1(id, label) VALUES (100, 'y'); SELECT 1 AS a, 2 AS b", 8, 2, $strict_on);
	func_test_mysqli_next_result($link, "DELETE FROM test_mysqli_next_result_table_1 WHERE id >= 100; SELECT 1 AS a; ", 11, 1, $strict_on);

	mysqli_close($link);

	var_dump(mysqli_next_result($link));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_next_result_table_1'; require_once("clean_table.inc");
?>