<?php
	function mysqli_field_seek_flags($flags) {

		$ret = '';

		if ($flags & MYSQLI_NOT_NULL_FLAG)
			$ret .= 'MYSQLI_NOT_NULL_FLAG ';

		if ($flags & MYSQLI_PRI_KEY_FLAG)
			$ret .= 'MYSQLI_PRI_KEY_FLAG ';

		if ($flags & MYSQLI_UNIQUE_KEY_FLAG)
			$ret .= 'MYSQLI_UNIQUE_KEY_FLAG ';

		if ($flags & MYSQLI_MULTIPLE_KEY_FLAG)
			$ret .= 'MYSQLI_MULTIPLE_KEY_FLAG ';

		if ($flags & MYSQLI_BLOB_FLAG)
			$ret .= 'MYSQLI_BLOB_FLAG ';

		if ($flags & MYSQLI_UNSIGNED_FLAG)
			$ret .= 'MYSQLI_UNSIGNED_FLAG ';

		if ($flags & MYSQLI_ZEROFILL_FLAG)
			$ret .= 'MYSQLI_ZEROFILL_FLAG ';

		if ($flags & MYSQLI_AUTO_INCREMENT_FLAG)
			$ret .= 'MYSQLI_AUTO_INCREMENT_FLAG ';

		if ($flags & MYSQLI_TIMESTAMP_FLAG)
			$ret .= 'MYSQLI_TIMESTAMP_FLAG ';

		if ($flags & MYSQLI_SET_FLAG)
			$ret .= 'MYSQLI_SET_FLAG ';

		if ($flags & MYSQLI_NUM_FLAG)
			$ret .= 'MYSQLI_NUM_FLAG ';

		if ($flags & MYSQLI_PART_KEY_FLAG)
			$ret .= 'MYSQLI_PART_KEY_FLAG ';

		if ($flags & MYSQLI_GROUP_FLAG)
			$ret .= 'MYSQLI_GROUP_FLAG ';

		return $ret;
	}

	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_field_seek_table_1'; require('table.inc');

	// Make sure that client, connection and result charsets are all the
	// same. Not sure whether this is strictly necessary.
	if (!mysqli_set_charset($link, 'utf8'))
		printf("[%d] %s\n", mysqli_errno($link), mysqli_errno($link));

	$charsetInfo = mysqli_get_charset($link);

	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_field_seek_table_1 ORDER BY id LIMIT 1", MYSQLI_USE_RESULT)) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_field_seek($res, -1));
	var_dump(mysqli_fetch_field($res));
	var_dump(mysqli_field_seek($res, 0));
	var_dump(mysqli_fetch_field($res));
	var_dump(mysqli_field_seek($res, 1));

	$field = mysqli_fetch_field($res);
	var_dump($field);
	/* label column, result set charset */
	if ($field->charsetnr != $charsetInfo->number) {
		printf("[004] Expecting charset %s/%d got %d\n",
			$charsetInfo->charset, $charsetInfo->number, $field->charsetnr);
	}
	if ($field->length != $charsetInfo->max_length) {
		printf("[005] Expecting length %d got %d\n",
			$charsetInfo->max_length, $field->max_length);
	}

	var_dump(mysqli_field_tell($res));
	var_dump(mysqli_field_seek($res, 2));
	var_dump(mysqli_fetch_field($res));
	var_dump(mysqli_field_seek($res, PHP_INT_MAX + 1));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT NULL as _null", MYSQLI_STORE_RESULT)) {
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	var_dump(mysqli_field_seek($res, 0));
	var_dump(mysqli_fetch_field($res));

	mysqli_free_result($res);

	var_dump(mysqli_field_seek($res, 0));

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_field_seek_table_1'; require_once("clean_table.inc");
?>