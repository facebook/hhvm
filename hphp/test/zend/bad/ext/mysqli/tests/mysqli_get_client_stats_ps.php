<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_get_client_stats_ps_table_1'; require_once('table.inc');

	$stats = mysqli_get_client_stats();
	printf("BEGINNING: rows_fetched_from_client_ps_unbuffered = %d\n",	$stats['rows_fetched_from_client_ps_unbuffered']);
	printf("BEGINNING: rows_fetched_from_client_ps_buffered = %d\n",	$stats['rows_fetched_from_client_ps_buffered']);
	printf("BEGINNING: rows_fetched_from_client_ps_cursor = %d\n",	$stats['rows_fetched_from_client_ps_cursor']);

	if (!$stmt = mysqli_stmt_init($link))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$id = null;
	if (!mysqli_stmt_prepare($stmt, 'SELECT id FROM test') ||
			!mysqli_stmt_execute($stmt) ||
			!mysqli_stmt_store_result($stmt) ||
			!mysqli_stmt_bind_result($stmt, $id))
		printf("[002] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$num_rows = 0;
	while (mysqli_stmt_fetch($stmt))
		$num_rows++;

	mysqli_stmt_free_result($stmt);

	$after = mysqli_get_client_stats();

	if ($after['rows_fetched_from_client_ps_unbuffered'] != $stats['rows_fetched_from_client_ps_unbuffered'])
		printf("[003] Unbuffered rows got increased after buffered PS, expecting %d got %d.\n",
			$stats['rows_fetched_from_client_ps_unbuffered'],
			$after['rows_fetched_from_client_ps_unbuffered']);

	$stats['rows_fetched_from_client_ps_buffered'] += $num_rows;
	if ($after['rows_fetched_from_client_ps_buffered'] != $stats['rows_fetched_from_client_ps_buffered'] )
		printf("[005] Buffered rows should be %d got %d.\n",
			$stats['rows_fetched_from_client_ps_buffered'],
			$after['rows_fetched_from_client_ps_buffered']);

	$stats = $after;
	printf("BUFFERED: rows_fetched_from_client_ps_unbuffered = %d\n",	$stats['rows_fetched_from_client_ps_unbuffered']);
	printf("BUFFERED: rows_fetched_from_client_ps_buffered = %d\n",	$stats['rows_fetched_from_client_ps_buffered']);
	printf("BUFFERED: rows_fetched_from_client_ps_cursor = %d\n",	$stats['rows_fetched_from_client_ps_cursor']);

	$id = null;
	if (!mysqli_stmt_prepare($stmt, 'SELECT id FROM test') ||
			!mysqli_stmt_execute($stmt) ||
			!mysqli_stmt_bind_result($stmt, $id))
		printf("[006] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$num_rows = 0;
	while (mysqli_stmt_fetch($stmt))
		$num_rows++;

	$after = mysqli_get_client_stats();
	$stats['rows_fetched_from_client_ps_unbuffered'] += $num_rows;
	if ($after['rows_fetched_from_client_ps_unbuffered'] != $stats['rows_fetched_from_client_ps_unbuffered'])
		printf("[007] Unbuffered rows should be %d got %d.\n",
			$stats['rows_fetched_from_client_ps_unbuffered'],
			$after['rows_fetched_from_client_ps_unbuffered']);

	if ($after['rows_fetched_from_client_ps_buffered'] != $stats['rows_fetched_from_client_ps_buffered'] )
		printf("[005] Buffered rows should be unchanged, expecting %d got %d.\n",
			$stats['rows_fetched_from_client_ps_buffered'],
			$after['rows_fetched_from_client_ps_buffered']);

	mysqli_stmt_free_result($stmt);
	mysqli_stmt_close($stmt);

	$stats = $after;
	printf("UNBUFFERED: rows_fetched_from_client_ps_unbuffered = %d\n",	$stats['rows_fetched_from_client_ps_unbuffered']);
	printf("UNBUFFERED: rows_fetched_from_client_ps_buffered = %d\n",	$stats['rows_fetched_from_client_ps_buffered']);
	printf("UNBUFFERED: rows_fetched_from_client_ps_cursor = %d\n",	$stats['rows_fetched_from_client_ps_cursor']);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_get_client_stats_ps_table_1'; require_once("clean_table.inc");
?>