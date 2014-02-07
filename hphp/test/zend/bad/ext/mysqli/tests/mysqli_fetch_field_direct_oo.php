<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$mysqli = new mysqli();
	$res = @new mysqli_result($mysqli);
	$test_table_name = 'test_mysqli_fetch_field_direct_oo_table_1'; require('table.inc');

	if (!$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	if (!$res = $mysqli->query("SELECT id AS ID, label FROM test_mysqli_fetch_field_direct_oo_table_1 AS TEST ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	var_dump($res->fetch_field_direct(-1));
	var_dump($res->fetch_field_direct(0));
	var_dump($res->fetch_field_direct(2));

	$res->free_result();

	if (NULL !== ($tmp = $res->fetch_field_direct(0)))
		printf("[007] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$mysqli->close();
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_field_direct_oo_table_1'; require_once("clean_table.inc");
?>