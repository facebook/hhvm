<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	// Note: no SQL type tests, internally the same function gets used as for mysqli_fetch_array() which does a lot of SQL type test
	$test_table_name = 'test_mysqli_fetch_assoc_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_fetch_assoc_table_1 ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	print "[005]\n";
	var_dump(mysqli_fetch_assoc($res));

	print "[006]\n";
	var_dump(mysqli_fetch_assoc($res));

	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT
		1 AS a,
		2 AS a,
		3 AS c,
		4 AS C,
		NULL AS d,
		true AS e,
		5 AS '-1',
		6 AS '-10',
		7 AS '-100',
		8 AS '-1000',
		9 AS '10000',
		'a' AS '100000',
		'b' AS '1000000',
		'c' AS '9',
		'd' AS '9',
		'e' AS '01',
		'f' AS '-02'
	")) {
		printf("[007] Cannot run query, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[008]\n";
	var_dump(mysqli_fetch_assoc($res));

	mysqli_free_result($res);

	if (NULL !== ($tmp = mysqli_fetch_assoc($res)))
		printf("[008] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_assoc_table_1'; require_once("clean_table.inc");
?>