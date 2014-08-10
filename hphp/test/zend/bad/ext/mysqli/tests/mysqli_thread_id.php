<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_thread_id_table_1'; require('table.inc');

	if (!is_int($tmp = mysqli_thread_id($link)) || (0 === $tmp))
		printf("[003] Expecting int/any but zero, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_errno($link), mysqli_error($link));

	// should work if the thread id is correct
	mysqli_kill($link, mysqli_thread_id($link));

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_thread_id($link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_thread_id_table_1'; require_once("clean_table.inc");
?>