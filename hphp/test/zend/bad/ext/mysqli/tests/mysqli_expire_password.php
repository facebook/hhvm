<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_expire_password_table_1'; require_once('table.inc');

	/* default */
	if (!$link = my_mysqli_connect($host, 'expiretest', "", $db, $port, $socket)) {
		printf("[001] Cannot connect [%d] %s\n",
			mysqli_connect_errno(), mysqli_connect_error());
	} else {
		$link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1");
		printf("[002] Connect should fail, [%d] %s\n", $link->errno, $link->error);
	}

	/* explicitly requesting default */
	$link = mysqli_init();
	$link->options(MYSQLI_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, 0);
	if (!my_mysqli_real_connect($link, $host, 'expiretest', "", $db, $port, $socket)) {
		printf("[003] Cannot connect [%d] %s\n",
			mysqli_connect_errno(), mysqli_connect_error());
	} else {
		$link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1");
		printf("[004] Connect should fail, [%d] %s\n", $link->errno, $link->error);
	}

	/* allow connect */
	$link = mysqli_init();
	$link->options(MYSQLI_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, 1);
	if (!my_mysqli_real_connect($link, $host, 'expiretest', "", $db, $port, $socket)) {
		printf("[005] Cannot connect [%d] %s\n",
			mysqli_connect_errno(), mysqli_connect_error());
	} else {
		$link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1");
		printf("[006] Connect allowed, query fail, [%d] %s\n", $link->errno, $link->error);
		$link->close();
	}

	/* allow connect, fix pw */
	$link = mysqli_init();
	$link->options(MYSQLI_OPT_CAN_HANDLE_EXPIRED_PASSWORDS, 1);
	if (!my_mysqli_real_connect($link, $host, 'expiretest', "", $db, $port, $socket)) {
		printf("[007] Cannot connect [%d] %s\n",
			mysqli_connect_errno(), mysqli_connect_error());
	} else {
		$link->query("SET PASSWORD=PASSWORD('expiretest')");
		printf("[008] Connect allowed, pw set, [%d] %s\n", $link->errno, $link->error);
		if ($res = $link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1"))
			var_dump($res->fetch_assoc());
		$link->close();
	}


	/* check login */
	if (!$link = my_mysqli_connect($host, 'expiretest', "expiretest", $db, $port, $socket)) {
		printf("[001] Cannot connect [%d] %s\n",
			mysqli_connect_errno(), mysqli_connect_error());
	} else {
		$link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1");
		if ($res = $link->query("SELECT id FROM test_mysqli_expire_password_table_1 WHERE id = 1"))
			var_dump($res->fetch_assoc());
		$link->close();
	}



	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_expire_password_table_1'; require_once("clean_table.inc");
	mysqli_query($link, 'DROP USER expiretest');
	mysqli_query($link, 'DROP USER expiretest@localhost');
?>