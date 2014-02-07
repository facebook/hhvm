<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_data_seek_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, 'SELECT * FROM test_mysqli_data_seek_table_1 ORDER BY id LIMIT 4', MYSQLI_STORE_RESULT))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (true !== ($tmp = mysqli_data_seek($res, 3)))
		printf("[005] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	$row = mysqli_fetch_assoc($res);
	if (4 != $row['id'])
		printf("[006] Expecting record 4/d, got record %s/%s\n", $row['id'], $row['label']);

	if (true !== ($tmp = mysqli_data_seek($res, 0)))
		printf("[007] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	$row = mysqli_fetch_assoc($res);
	if (1 != $row['id'])
		printf("[008] Expecting record 1/a, got record %s/%s\n", $row['id'], $row['label']);

	if (false !== ($tmp = mysqli_data_seek($res, 4)))
		printf("[009] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (false !== ($tmp = mysqli_data_seek($res, -1)))
		printf("[010] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_free_result($res);

	if (!$res = mysqli_query($link, 'SELECT * FROM test_mysqli_data_seek_table_1 ORDER BY id', MYSQLI_USE_RESULT))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (false !== ($tmp = mysqli_data_seek($res, 3)))
		printf("[012] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_free_result($res);

	if (NULL !== ($tmp = mysqli_data_seek($res, 1)))
		printf("[013] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_data_seek_table_1'; require_once("clean_table.inc");
?>