<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link,"DROP TABLE IF EXISTS test_057_table_1");
	mysqli_query($link,"CREATE TABLE test_057_table_1 (a int)");

	mysqli_query($link, "INSERT INTO test_057_table_1 VALUES (1),(2),(3)");

	$stmt = mysqli_prepare($link, "SELECT * FROM test_057_table_1");
	mysqli_stmt_execute($stmt);

	/* this should produce an out of sync error */
	if ($result = mysqli_query($link, "SELECT * FROM test_057_table_1")) {
		mysqli_free_result($result);
		printf ("Query ok\n");
	}
	mysqli_stmt_close($stmt);

	/* now we should try mysqli_stmt_reset() */
	$stmt = mysqli_prepare($link, "SELECT * FROM test_057_table_1");
	var_dump(mysqli_stmt_execute($stmt));
	var_dump(mysqli_stmt_reset($stmt));

	var_dump($stmt = mysqli_prepare($link, "SELECT * FROM test_057_table_1"));
	if ($stmt->affected_rows !== 0)
			printf("[001] Expecting 0, got %d\n", $stmt->affected_rows);

	var_dump(mysqli_stmt_execute($stmt));
	var_dump($stmt = @mysqli_prepare($link, "SELECT * FROM test_057_table_1"), mysqli_error($link));
	var_dump(mysqli_stmt_reset($stmt));

	$stmt = mysqli_prepare($link, "SELECT * FROM test_057_table_1");
	mysqli_stmt_execute($stmt);
	$result1 = mysqli_stmt_result_metadata($stmt);
	mysqli_stmt_store_result($stmt);

	printf ("Rows: %d\n", mysqli_stmt_affected_rows($stmt));

	/* this should show an error, cause results are not buffered */
	if ($result = mysqli_query($link, "SELECT * FROM test_057_table_1")) {
		$row = mysqli_fetch_row($result);
		mysqli_free_result($result);
	}

	var_dump($row);

	mysqli_free_result($result1);
	mysqli_stmt_close($stmt);
	mysqli_close($link);
	echo "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_057_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>