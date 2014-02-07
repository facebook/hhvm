<?php
	include ("connect.inc");

	$link = mysqli_init();
	if (!mysqli_options($link, MYSQLI_INIT_COMMAND, "SELECT 1")) {
		printf("[001] Cannot set INIT_COMMAND\n");
	}

	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[002] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	var_dump($link->query("SELECT 42")->fetch_row());

	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_bug49027_table_1") ||
		!mysqli_query($link, sprintf("CREATE TABLE test_bug49027_table_1(id INT) ENGINE=%s", $engine))) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_close($link);

	$link = mysqli_init();
	if (!mysqli_options($link, MYSQLI_INIT_COMMAND, "INSERT INTO test_bug49027_table_1(id) VALUES(1)")) {
		printf("[004] Cannot set INIT_COMMAND\n");
	}

	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[005] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!$res = mysqli_query($link, "SELECT id FROM test_bug49027_table_1"))
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	var_dump(mysqli_fetch_assoc($res));

	mysqli_free_result($res);
	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_bug49027_table_1'; require_once("clean_table.inc");
?>