<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	if (!is_null($tmp = @mysqli_fetch_row()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_fetch_row($link)))
		printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$test_table_name = 'test_mysqli_fetch_row_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id, label, id AS _id FROM test_mysqli_fetch_row_table_1 ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	print "[004]\n";
	var_dump(mysqli_fetch_row($res));

	print "[005]\n";
	var_dump(mysqli_fetch_row($res));

	mysqli_free_result($res);

	var_dump(mysqli_fetch_row($res));

	mysqli_close($link);
	print "done!";
?>
<?php
	require_once("clean_table.inc");
?>