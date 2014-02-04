<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_change_user_rollback_table_1'; require_once('table.inc');

	if (!mysqli_query($link, 'ALTER TABLE test_mysqli_change_user_rollback_table_1 ENGINE=InnoDB'))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_autocommit($link, false);

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test_mysqli_change_user_rollback_table_1'))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_free_result($res);

	$num = $row['_num'];
	assert($num > 0);

	if (!$res = mysqli_query($link, 'DELETE FROM test_mysqli_change_user_rollback_table_1'))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test_mysqli_change_user_rollback_table_1'))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_free_result($res);

	if (0 != $row['_num'])
		printf("[007] Rows should have been deleted in this transaction\n");

	// DELETE should be rolled back
	mysqli_change_user($link, $user, $passwd, $db);

	if (!$res = mysqli_query($link, 'SELECT COUNT(*) AS _num FROM test_mysqli_change_user_rollback_table_1'))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$row = mysqli_fetch_assoc($res))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if ($row['_num'] != $num)
		printf("[010] Expecting %d rows in the table test_mysqli_change_user_rollback_table_1, found %d rows\n",
			$num, $row['_num']);

	mysqli_free_result($res);
	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_change_user_rollback_table_1'; require_once("clean_table.inc");
?>