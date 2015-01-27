<?php
	require('connect.inc');
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
		die();
	}


	if (!mysqli_query($link, "DROP TABLE IF EXISTS test")) {
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}

	if (!mysqli_query($link, "CREATE TABLE test(id INT PRIMARY KEY, fp4 FLOAT, fp8 DOUBLE) ENGINE = InnoDB")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}

	// Insert via string to make sure the real floating number gets to the DB
	if (!mysqli_query($link, "INSERT INTO test(id, fp4, fp8) VALUES (1, 9.9999, 9.9999)")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}

	if (!($stmt = mysqli_prepare($link, "SELECT id, fp4, fp8 FROM test"))) {
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}

	if (!mysqli_stmt_execute($stmt)) {
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}


	if (!($result = mysqli_stmt_get_result($stmt))) {
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		die();
	}

	$data = mysqli_fetch_assoc($result);
	print $data['id'] . ": " . $data['fp4'] . ": " . $data['fp8'] . "\n";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_bug67839_table_1'; require_once("clean_table.inc");
?>