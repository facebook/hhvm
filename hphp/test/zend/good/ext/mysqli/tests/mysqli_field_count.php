<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_field_count_table_1'; require('table.inc');

	var_dump(mysqli_field_count($link));

	if (!$res = mysqli_query($link, "SELECT * FROM test_mysqli_field_count_table_1 ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_field_count($link));

	mysqli_free_result($res);

	if (!mysqli_query($link, "INSERT INTO test_mysqli_field_count_table_1(id, label) VALUES (100, 'x')"))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	var_dump($link->field_count);
	var_dump(mysqli_field_count($link));

	if (!$res = mysqli_query($link, "SELECT NULL as _null, '' AS '', 'three' AS 'drei'"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	var_dump(mysqli_field_count($link));
	mysqli_free_result($res);

	mysqli_close($link);

	var_dump(mysqli_field_count($link));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_field_count_table_1'; require_once("clean_table.inc");
?>