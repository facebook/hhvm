<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_get_client_stats_implicit_free_table_1'; require_once('table.inc');

	$stats = mysqli_get_client_stats();
	printf("BEGINNING: implicit_free_result = %d\n",	$stats['implicit_free_result']);

	if (!$res = mysqli_query($link, 'SELECT id FROM test'))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($res);
	mysqli_close($link);

	$after = mysqli_get_client_stats();
	if ($after['implicit_free_result'] != $stats['implicit_free_result'])
		printf("[002] Where is the missing mysqli_free_result() call? implicit_free_result has changed by %d.\n",
			$after['implicit_free_result'] - $stats['implicit_free_result']);

	$stats = $after;
	printf("END: implicit_free_result = %d\n",	$stats['implicit_free_result']);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_get_client_stats_implicit_free_table_1'; require_once("clean_table.inc");
?>