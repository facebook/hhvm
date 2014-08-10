<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_get_client_stats_skipped_table_1'; require_once('table.inc');

	if (!$res = mysqli_query($link, 'SELECT id FROM test_mysqli_get_client_stats_skipped_table_1', MYSQLI_STORE_RESULT))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$num_rows = mysqli_num_rows($res);
	assert($num_rows > 2);
	mysqli_free_result($res);

	$before = mysqli_get_client_stats();
	printf("BEFORE: rows_skipped_normal = %d\n", $before['rows_skipped_normal']);

	if (!$res = mysqli_query($link, 'SELECT id FROM test_mysqli_get_client_stats_skipped_table_1', MYSQLI_USE_RESULT))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	/* fetch all rows but the last one */
	for ($i = 0; $i < $num_rows - 1; $i++)
		$row = mysqli_fetch_assoc($res);

	/* enforce implicit cleaning of the wire and skipping the last row */
	mysqli_free_result($res);
	$after = mysqli_get_client_stats();
	printf("AFTER: rows_skipped_normal = %d\n", $after['rows_skipped_normal']);

	if ($after['rows_skipped_normal'] != $before['rows_skipped_normal'] + 1)
		printf("Statistics should show an increase of 1 for rows_skipped_normal, ".
				"but before=%d after=%d\n", $before['rows_skipped_normal'], $after['rows_skipped_normal']);

	mysqli_close($link);
	print "done!";
?>