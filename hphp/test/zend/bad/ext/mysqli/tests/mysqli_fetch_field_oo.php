<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	// Note: no SQL type tests, internally the same function gets used as for mysqli_fetch_array() which does a lot of SQL type test
	$mysqli = new mysqli();
	$res = @new mysqli_result($mysqli);
	$test_table_name = 'test_mysqli_fetch_field_oo_table_1'; require('table.inc');
	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	// Make sure that client, connection and result charsets are all the
	// same. Not sure whether this is strictly necessary.
	if (!$mysqli->set_charset('utf8'))
		printf("[%d] %s\n", $mysqli->errno, $mysqli->errno);

	$charsetInfo = $mysqli->get_charset();

	if (!$res = $mysqli->query("SELECT id AS ID, label FROM test_mysqli_fetch_field_oo_table_1 AS TEST ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	var_dump($res->fetch_field());

	$tmp = $res->fetch_field();
	var_dump($tmp);
	if ($tmp->charsetnr != $charsetInfo->number) {
		printf("[005] Expecting charset %s/%d got %d\n",
			$charsetInfo->charset, $charsetInfo->number, $tmp->charsetnr);
	}
	if ($tmp->length != $charsetInfo->max_length) {
		printf("[006] Expecting length %d got %d\n",
			$charsetInfo->max_length, $tmp->max_length);
	}
	if ($tmp->db != $db) {
		printf("[007] Expecting database '%s' got '%s'\n",
		  $db, $tmp->db);
	}

	var_dump($res->fetch_field());

	$res->free_result();

	if (NULL !== ($tmp = $res->fetch_field()))
		printf("[007] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$mysqli->close();
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_field_oo_table_1'; require_once("clean_table.inc");
?>