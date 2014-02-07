<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	// Note: no SQL type tests, internally the same function gets used as for mysqli_fetch_array() which does a lot of SQL type test
	$test_table_name = 'test_mysqli_fetch_fields_table_1'; require('table.inc');

	// Make sure that client, connection and result charsets are all the
	// same. Not sure whether this is strictly necessary.
	if (!mysqli_set_charset($link, 'utf8'))
		printf("[%d] %s\n", mysqli_errno($link), mysqli_errno($link));

	$charsetInfo = mysqli_get_charset($link);

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_fields_table_1 AS TEST ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	$fields = mysqli_fetch_fields($res);
	foreach ($fields as $k => $field) {
		var_dump($field);
		switch ($k) {
			case 1:
				/* label column, result set charset */
				if ($field->charsetnr != $charsetInfo->number) {
					printf("[004] Expecting charset %s/%d got %d\n",
						$charsetInfo->charset,
						$charsetInfo->number, $field->charsetnr);
				}
				if ($field->length != $charsetInfo->max_length) {
					printf("[005] Expecting length %d got %d\n",
						$charsetInfo->max_length,
						$field->max_length);
				}
				break;
		}
	}

	mysqli_free_result($res);

	if (NULL !== ($tmp = mysqli_fetch_fields($res)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_fields_table_1'; require_once("clean_table.inc");
?>