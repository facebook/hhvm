<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	// Note: no SQL type tests, internally the same function gets used as for mysqli_fetch_array() which does a lot of SQL type test
	$mysqli = new mysqli();
	$res = @new mysqli_result($mysqli);
	$test_table_name = 'test_mysqli_fetch_assoc_oo_table_1'; require('table.inc');
	if (!$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	if (!$res = $mysqli->query("SELECT id, label FROM test_mysqli_fetch_assoc_oo_table_1 ORDER BY id LIMIT 1")) {
		printf("[004] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	print "[005]\n";
	var_dump($res->fetch_assoc());

	print "[006]\n";
	var_dump($res->fetch_assoc());

	$res->free_result();

	if (!$res = $mysqli->query("SELECT 1 AS a, 2 AS a, 3 AS c, 4 AS C, NULL AS d, true AS e")) {
		printf("[007] Cannot run query, [%d] %s\n", $mysqli->errno, $mysqli->error);
	}
	print "[008]\n";
	var_dump($res->fetch_assoc());

	$res->free_result();

	if (NULL !== ($tmp = $res->fetch_assoc()))
		printf("[008] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_assoc_oo_table_1'; require_once("clean_table.inc");
?>