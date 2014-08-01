<?php
	$before = mysqli_get_client_stats();
	if (!is_array($before) || empty($before)) {
		printf("[001] Expecting non-empty array, got %s.\n", gettype($before));
		var_dump($before);
	}

	// connect and table inc connect to mysql and create tables
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_get_client_stats_off_table_1'; require_once('table.inc');
	$after = mysqli_get_client_stats();

	if ($before !== $after) {
		printf("[002] Statistics have changed\n");
		var_dump($before);
		var_dump($after);
	}

	foreach ($after as $k => $v)
		if ($v != 0) {
			printf("[003] Field %s should not have any other value but 0, got %s.\n",
				$k, $v);
		}

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_get_client_stats_off_table_1'; require_once("clean_table.inc");
?>