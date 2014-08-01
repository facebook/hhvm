<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_warning_count_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_warning_count_table_1"))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (0 !== ($tmp = mysqli_warning_count($link)))
		printf("[005] Expecting int/0, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_query($link, "DROP TABLE IF EXISTS this_table_does_not_exist"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (1 !== ($tmp = mysqli_warning_count($link)))
		printf("[007] Expecting int/1, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_warning_count($link)))
		printf("[010] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_warning_count_table_1'; require_once("clean_table.inc");
?>