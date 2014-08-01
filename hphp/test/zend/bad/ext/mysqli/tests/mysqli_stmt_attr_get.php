<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_attr_get_table_1'; require('table.inc');
	$valid_attr = array("max_length" => MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH);
	if (mysqli_get_client_version() > 50003)
		$valid_attr["cursor_type"] = MYSQLI_STMT_ATTR_CURSOR_TYPE;

	if ($IS_MYSQLND && mysqli_get_client_version() > 50007)
		$valid_attr["prefetch_rows"] = MYSQLI_STMT_ATTR_PREFETCH_ROWS;

	do {
		$invalid_attr = mt_rand(0, 10000);
	} while (in_array($invalid_attr, $valid_attr));

	$stmt = mysqli_stmt_init($link);
	mysqli_stmt_prepare($stmt, 'SELECT * FROM test_mysqli_stmt_attr_get_table_1');
	foreach ($valid_attr as $k => $attr) {
		if (false === ($tmp = mysqli_stmt_attr_get($stmt, $attr))) {
			printf("[006] Expecting any type, but not boolean/false, got %s/%s for attribute %s/%s\n",
				gettype($tmp), $tmp, $k, $attr);
		}
	}

	$stmt->close();

	foreach ($valid_attr as $k => $attr) {
		if (!is_null($tmp = @mysqli_stmt_attr_get($stmt, $attr))) {
			printf("[007] Expecting NULL/NULL, got %s/%s for attribute %s/%s\n",
				gettype($tmp), $tmp, $k, $attr);
		}
	}

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_attr_get_table_1'; require_once("clean_table.inc");
?>