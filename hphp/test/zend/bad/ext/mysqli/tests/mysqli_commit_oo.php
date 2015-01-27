<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$mysqli = new mysqli();
	if (!is_null($tmp = @$mysqli->commit())) {
		printf("[013] Expecting NULL got %s/%s\n", gettype($tmp), $tmp);
	}

	if (!$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	}

	if (true !== ($tmp = $mysqli->commit())) {
		printf("[002] Expecting boolean/true got %s/%s\n", gettype($tmp), $tmp);
	}

	if (true !== ($tmp = $mysqli->autocommit(false))) {
		printf("[003] Cannot turn off autocommit, expecting true, got %s/%s\n", gettype($tmp), $tmp);
	}

	if (!$mysqli->query('DROP TABLE IF EXISTS test_mysqli_commit_oo_table_1')) {
		printf("[004] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	if (!$mysqli->query('CREATE TABLE test_mysqli_commit_oo_table_1(id INT) ENGINE = InnoDB')) {
		printf("[005] Cannot create test table, [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	if (!$mysqli->query('INSERT INTO test_mysqli_commit_oo_table_1(id) VALUES (1)')) {
		printf("[006] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	$tmp = $mysqli->commit();
	if ($tmp !== true) {
		printf("[007] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);
	}

	if (!$mysqli->query('ROLLBACK'))
		printf("[008] [%d] %s\n", $mysqli->errno, $mysqli->error);

	if (!$res = $mysqli->query('SELECT COUNT(*) AS num FROM test_mysqli_commit_oo_table_1')) {
		printf("[009] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	$tmp = $res->fetch_assoc();
	if (1 != $tmp['num']) {
		printf("[010] Expecting 1 row in table test, found %d rows\n", $tmp['num']);
	}
	$res->free();

	if (!$mysqli->query('DROP TABLE IF EXISTS test_mysqli_commit_oo_table_1')) {
		printf("[011] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	if (!$mysqli->commit(0 , "tx_name0123")) {
		printf("[012] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}
	if (!$mysqli->commit(0 , "*/ nonsense")) {
		printf("[013] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}
	if (!$mysqli->commit(0 , "tx_name ulf вендел")) {
		printf("[014] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}
	if (!$mysqli->commit(0 , "tx_name \t\n\r\b")) {
		printf("[015] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}
	if (!$mysqli->commit(MYSQLI_TRANS_COR_AND_CHAIN | MYSQLI_TRANS_COR_NO_RELEASE , "tx_name")) {
		printf("[016] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	$mysqli->close();

	if (NULL !== ($tmp = @$mysqli->commit())) {
		printf("[017] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);
	}

	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_commit_oo_table_1'; require_once("clean_table.inc");
?>