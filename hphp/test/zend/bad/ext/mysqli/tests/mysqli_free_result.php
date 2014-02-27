<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_free_result_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id FROM test_mysqli_free_result_table_1 ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	print "a\n";
	var_dump(mysqli_free_result($res));
	print "b\n";
	var_dump(mysqli_free_result($res));

	if (!$res = mysqli_query($link, "SELECT id FROM test_mysqli_free_result_table_1 ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "c\n";
	var_dump($res = mysqli_store_result($link));
	var_dump(mysqli_error($link));
	print "[005]\n";
	var_dump(mysqli_free_result($res));

	if (!$res = mysqli_query($link, "SELECT id FROM test_mysqli_free_result_table_1 ORDER BY id LIMIT 1")) {
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "d\n";
	var_dump($res = mysqli_use_result($link));
	var_dump(mysqli_error($link));
	print "[007]\n";
	var_dump(mysqli_free_result($res));

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_free_result_table_1'; require_once("clean_table.inc");
?>