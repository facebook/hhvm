<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_prepare_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_prepare_table_1", MYSQLI_USE_RESULT))
		printf("[004] [%d] %s, next test will fail\n", mysqli_errno($link), mysqli_error($link));

	if (false !== ($tmp = mysqli_prepare($link, 'SELECT id FROM test_mysqli_prepare_table_1 WHERE id > ?')))
		printf("[005] Expecting boolean/false, got %s, [%d] %s\n", gettype($tmp), mysqli_errno($link), mysqli_error($link));

	mysqli_free_result($res);

	if (!is_object(($stmt = mysqli_prepare($link, 'SELECT id FROM test_mysqli_prepare_table_1'))) || !mysqli_stmt_execute($stmt))
		printf("[006][%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);


	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_prepare_table_2"))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_object(($stmt = mysqli_prepare($link, 'CREATE TABLE test_mysqli_prepare_table_2(id INT) ENGINE =' . $engine))) || !mysqli_stmt_execute($stmt))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);


	if (!is_object(($stmt = mysqli_prepare($link, 'INSERT INTO test_mysqli_prepare_table_2(id) VALUES(?)'))))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$id = 1;
	if (!mysqli_stmt_bind_param($stmt, 'i', $id) || !mysqli_stmt_execute($stmt))
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, 'REPLACE INTO test_mysqli_prepare_table_2(id) VALUES (?)'))))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$id = 2;
	if (!mysqli_stmt_bind_param($stmt, 'i', $id) || !mysqli_stmt_execute($stmt))
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, 'UPDATE test_mysqli_prepare_table_2 SET id = ? WHERE id = ?'))))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$id = 3;
	$where = 2;
	if (!mysqli_stmt_bind_param($stmt, 'ii', $id, $where) || !mysqli_stmt_execute($stmt))
		printf("[014] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, 'DELETE FROM test_mysqli_prepare_table_2 WHERE id = ?'))))
		printf("[015] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$where = 3;
	if (!mysqli_stmt_bind_param($stmt, 'i', $where) || !mysqli_stmt_execute($stmt))
		printf("[016] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, 'SET @test_mysqli_prepare_var_1 = ?'))))
		printf("[017] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$test_mysqli_prepare_var_1 = 'test_mysqli_prepare_var_1';
	if (!mysqli_stmt_bind_param($stmt, 's', $test_mysqli_prepare_var_1) || !mysqli_stmt_execute($stmt))
		printf("[018] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, "DO GET_LOCK('test_mysqli_prepare_lock_1', 1)"))))
		printf("[019] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	mysqli_stmt_close($stmt);

	if (!is_object(($stmt = mysqli_prepare($link, 'SELECT id, @test_mysqli_prepare_var_1 FROM test_mysqli_prepare_table_2'))))
		printf("[020] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$id = $test_mysqli_prepare_var_1 = null;
	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_bind_result($stmt, $id, $test_mysqli_prepare_var_1))
		printf("[021] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	while (mysqli_stmt_fetch($stmt)) {
		if (('test_mysqli_prepare_var_1' !== $test_mysqli_prepare_var_1) || (1 !== $id))
			printf("[022] Expecting 'test_mysqli_prepare_var_1'/1, got %s/%s. [%d] %s\n",
				$test_mysqli_prepare_var_1, $id, mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	var_dump(mysqli_stmt_prepare($stmt, 'SELECT 1; SELECT 2'));

	mysqli_stmt_close($stmt);
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_prepare_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_prepare_table_2"))
	printf("[c003] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>