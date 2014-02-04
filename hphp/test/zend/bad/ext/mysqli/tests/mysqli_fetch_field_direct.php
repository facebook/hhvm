<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_fetch_field_direct_table_1'; require('table.inc');

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_field_direct_table_1 AS TEST ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_fetch_field_direct($res, -1));
	var_dump(mysqli_fetch_field_direct($res, 0));
	var_dump(mysqli_fetch_field_direct($res, 2));

	mysqli_free_result($res);

	if (NULL !== ($tmp = mysqli_fetch_field_direct($res, 0)))
		printf("Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_field_direct_table_1'; require_once("clean_table.inc");
?>