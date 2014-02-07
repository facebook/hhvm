<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	// Note: no SQL type tests, internally the same function gets used as for mysqli_fetch_array() which does a lot of SQL type test
	$test_table_name = 'test_mysqli_fetch_field_table_1'; require('table.inc');

	// Make sure that client, connection and result charsets are all the
	// same. Not sure whether this is strictly necessary.
	if (!mysqli_set_charset($link, 'utf8'))
		printf("[%d] %s\n", mysqli_errno($link), mysqli_errno($link));

	$charsetInfo = mysqli_get_charset($link);

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_field_table_1 AS TEST ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	/* ID column, binary charset */
	$tmp = mysqli_fetch_field($res);
	var_dump($tmp);

	/* label column, result set charset */
	$tmp = mysqli_fetch_field($res);
	var_dump($tmp);
	if ($tmp->charsetnr != $charsetInfo->number) {
		printf("[004] Expecting charset %s/%d got %d\n",
			$charsetInfo->charset, $charsetInfo->number, $tmp->charsetnr);
	}
	if ($tmp->length != $charsetInfo->max_length) {
		printf("[005] Expecting length %d got %d\n",
			$charsetInfo->max_length, $tmp->max_length);
	}
	if ($tmp->db != $db) {
		printf("011] Expecting database '%s' got '%s'\n",
			$db, $tmp->db);
	}

	var_dump(mysqli_fetch_field($res));

	mysqli_free_result($res);

	// Read http://bugs.php.net/bug.php?id=42344 on defaults!
	if (NULL !== ($tmp = mysqli_fetch_field($res)))
		printf("[006] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_fetch_field_table_1"))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "CREATE TABLE test_mysqli_fetch_field_table_1(id INT NOT NULL DEFAULT 1)"))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "INSERT INTO test_mysqli_fetch_field_table_1(id) VALUES (2)"))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$res = mysqli_query($link, "SELECT id as _default_test_mysqli_fetch_field_table_1 FROM test_mysqli_fetch_field_table_1")) {
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	var_dump(mysqli_fetch_assoc($res));
	/* binary */
	var_dump(mysqli_fetch_field($res));
	mysqli_free_result($res);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_field_table_1'; require_once("clean_table.inc");
?>