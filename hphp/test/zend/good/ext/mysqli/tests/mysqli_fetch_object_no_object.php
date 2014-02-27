<?php
	$test_table_name = 'test_mysqli_fetch_object_no_object_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_no_object_table_1 AS TEST ORDER BY id LIMIT 5")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	$obj = mysqli_fetch_object($res);
	var_dump(gettype($obj));
	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_object_no_object_table_1'; require_once("clean_table.inc");
?>