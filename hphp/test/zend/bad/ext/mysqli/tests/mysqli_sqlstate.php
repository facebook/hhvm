<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_sqlstate_table_1'; require('table.inc');

	var_dump(@mysqli_sqlstate($link, "foo"));

	var_dump(mysqli_sqlstate($link));
	mysqli_query($link, "SELECT unknown_column FROM test_mysqli_sqlstate_table_1");
	var_dump(mysqli_sqlstate($link));
	mysqli_free_result(mysqli_query($link, "SELECT id FROM test_mysqli_sqlstate_table_1"));
	var_dump(mysqli_sqlstate($link));

	mysqli_close($link);

	var_dump(mysqli_sqlstate($link));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_sqlstate_table_1'; require_once("clean_table.inc");
?>