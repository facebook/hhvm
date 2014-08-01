<?php
	require_once("connect.inc");

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!$link->query("DROP TABLE IF EXISTS test_bug52891_table_1") ||
		!$link->query("DROP TABLE IF EXISTS test_bug52891_table_2")) {
		printf("[002] [%d] %s\n", $link->errno, $link->error);
	}

	if (!$link->query("CREATE TABLE test_bug52891_table_1(a BIGINT UNSIGNED) ENGINE=" . $engine) ||
		!$link->query("CREATE TABLE test_bug52891_table_2(a BIGINT) ENGINE=" . $engine)) {
		printf("[003] [%d] %s\n", $link->errno, $link->error);
	}


	if (!$stmt1 = $link->prepare("INSERT INTO test_bug52891_table_1 VALUES(?)"))
		printf("[004] [%d] %s\n", $link->errno, $link->error);

	if (!$stmt2 = $link->prepare("INSERT INTO test_bug52891_table_2 VALUES(?)"))
		printf("[005] [%d] %s\n", $link->errno, $link->error);

	$param = 42;

	if (!$stmt1->bind_param("i", $param))
		printf("[006] [%d] %s\n", $stmt1->errno, $stmt1->error);

	if (!$stmt2->bind_param("i", $param))
		printf("[007] [%d] %s\n", $stmt2->errno, $stmt2->error);

	/* first insert normal value to force initial send of types */
	if (!$stmt1->execute())
		printf("[008] [%d] %s\n", $stmt1->errno, $stmt1->error);

	if	(!$stmt2->execute())
		printf("[009] [%d] %s\n", $stmt2->errno, $stmt2->error);

	/* now try values that don't fit in long, on 32bit, new types should be sent or 0 will be inserted */
	$param = -4294967297;
	if (!$stmt2->execute())
		printf("[010] [%d] %s\n", $stmt2->errno, $stmt2->error);

	/* again normal value */
	$param = 43;

	if (!$stmt1->execute())
		printf("[011] [%d] %s\n", $stmt1->errno, $stmt1->error);

	if	(!$stmt2->execute())
		printf("[012] [%d] %s\n", $stmt2->errno, $stmt2->error);

	/* again conversion */
	$param = -4294967295;
	if (!$stmt2->execute())
		printf("[013] [%d] %s\n", $stmt2->errno, $stmt2->error);

	$param = 4294967295;
	if (!$stmt1->execute())
		printf("[014] [%d] %s\n", $stmt1->errno, $stmt1->error);

	if	(!$stmt2->execute())
		printf("[015] [%d] %s\n", $stmt2->errno, $stmt2->error);

	$param = 4294967297;
	if (!$stmt1->execute())
		printf("[016] [%d] %s\n", $stmt1->errno, $stmt1->error);

	if	(!$stmt2->execute())
		printf("[017] [%d] %s\n", $stmt2->errno, $stmt2->error);

	$result = $link->query("SELECT * FROM test_bug52891_table_2 ORDER BY a ASC");
	$result2 = $link->query("SELECT * FROM test_bug52891_table_1 ORDER BY a ASC");

	echo "test_bug52891_table_2:\n";
	while ($row = $result->fetch_assoc()) {
		var_dump($row);
	}
	echo "test_bug52891_table_1:\n";
	while ($row = $result2->fetch_assoc()) {
		var_dump($row);
	}

	echo "done";
?>
<?php
require_once('connect.inc');

if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
	printf("[clean] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);
}

if (!mysqli_query($link, 'DROP TABLE IF EXISTS test_bug52891_table_1')) {
	printf("[clean] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
}

if (!mysqli_query($link, 'DROP TABLE IF EXISTS test_bug52891_table_2')) {
	printf("[clean] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
}

mysqli_close($link);
?>