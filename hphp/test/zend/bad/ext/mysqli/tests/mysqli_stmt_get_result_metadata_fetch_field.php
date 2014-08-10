<?php
	$test_table_name = 'test_mysqli_stmt_get_result_metadata_fetch_field_table_1'; require('table.inc');

	// Make sure that client, connection and result charsets are all the
	// same. Not sure whether this is strictly necessary.
	if (!mysqli_set_charset($link, 'utf8'))
		printf("[%d] %s\n", mysqli_errno($link), mysqli_errno($link));

	$charsetInfo = mysqli_get_charset($link);

	if (!($stmt = mysqli_stmt_init($link)) ||
		!mysqli_stmt_prepare($stmt, "SELECT id, label, id + 1 as _id,  concat(label, '_') ___label FROM test_mysqli_stmt_get_result_metadata_fetch_field_table_1 ORDER BY id ASC LIMIT 3") ||
		!mysqli_stmt_execute($stmt))
		printf("[001] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!is_object($res = mysqli_stmt_get_result($stmt)) || 'mysqli_result' != get_class($res)) {
		printf("[002] Expecting object/mysqli_result got %s/%s, [%d] %s\n",
			gettype($res), $res, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	if (!is_object($res_meta = mysqli_stmt_result_metadata($stmt)) ||
		'mysqli_result' != get_class($res_meta)) {
		printf("[003] Expecting object/mysqli_result got %s/%s, [%d] %s\n",
			gettype($res), $res, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	$i = 0;
	while ($field = $res->fetch_field()) {
		var_dump($field);
		$i++;
		if (2 == $i) {
			/*
			Label column, result set charset.
			All of the following columns are "too hot" - too server dependent
			*/
			if ($field->charsetnr != $charsetInfo->number) {
				printf("[004] Expecting charset %s/%d got %d\n",
					$charsetInfo->charset,
					$charsetInfo->number, $field->charsetnr);
			}
			if ($field->length != $charsetInfo->max_length) {
				printf("[005] Expecting length %d got %d\n",
					$charsetInfo->max_length, $field->max_length);
			}
		}
	}

	mysqli_stmt_close($stmt);
	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_get_result_metadata_fetch_field_table_1'; require_once("clean_table.inc");
?>