<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_store_result_table_1'; require('table.inc');

	if (!$res = mysqli_real_query($link, "SELECT id, label FROM test_mysqli_store_result_table_1 ORDER BY id"))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_object($res = mysqli_store_result($link)))
		printf("[004] Expecting object, got %s/%s. [%d] %s\n",
			gettype($res), $res, mysqli_errno($link), mysqli_error($link));

	if (true !== ($tmp = mysqli_data_seek($res, 2)))
		printf("[005] Expecting boolean/true, got %s/%s. [%d] %s\n",
			gettype($tmp), $tmp, mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($res);

	if (!mysqli_query($link, "DELETE FROM test_mysqli_store_result_table_1"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (false !== ($res = mysqli_store_result($link)))
		printf("[007] Expecting boolean/false, got %s/%s. [%d] %s\n",
			gettype($res), $res, mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_store_result_table_1 ORDER BY id"))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (false !== ($tmp = mysqli_data_seek($res, 1)))
		printf("[009] Expecting boolean/false, got %s/%s\n",
			gettype($tmp), $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_store_result($link)))
		printf("[010] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_store_result_table_1'; require_once("clean_table.inc");
?>