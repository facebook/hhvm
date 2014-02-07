<?php
	require_once("connect.inc");

	if (!$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[001] Cannot connect\n");

	$test_table_name = 'test_mysqli_fetch_lengths_oo_table_1'; require('table.inc');
	if (!$res = $mysqli->query("SELECT id, label FROM test_mysqli_fetch_lengths_oo_table_1 ORDER BY id LIMIT 1")) {
		printf("[002] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	var_dump($res->lengths);
	while ($row = $res->fetch_assoc())
		var_dump($res->lengths);
	var_dump($res->lengths);

	$res->free_result();
	var_dump($res->lengths);
	$mysqli->close();
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_lengths_oo_table_1'; require_once("clean_table.inc");
?>
<?php
	$test_table_name = 'test_mysqli_fetch_lengths_oo_table_1'; require_once("clean_table.inc");
?>