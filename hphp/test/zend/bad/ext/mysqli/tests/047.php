<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link, "DROP TABLE IF EXISTS test_047_table_1");
	mysqli_query($link, "CREATE TABLE test_047_table_1 (foo int, bar varchar(10) character set latin1) ENGINE=" . $engine);

	mysqli_query($link, "INSERT INTO test_047_table_1 VALUES (1, 'Zak'),(2, 'Greant')");

	$stmt = mysqli_prepare($link, "SELECT * FROM test_047_table_1");
	mysqli_stmt_execute($stmt);
	$result = mysqli_stmt_result_metadata($stmt);

	echo "\n=== fetch_fields ===\n";
	var_dump(mysqli_fetch_fields($result));

	echo "\n=== fetch_field_direct ===\n";
	var_dump(mysqli_fetch_field_direct($result, 0));
	var_dump(mysqli_fetch_field_direct($result, 1));

	echo "\n=== fetch_field ===\n";
	while ($field = mysqli_fetch_field($result)) {
		var_dump($field);
	}

	print_r(mysqli_fetch_lengths($result));

	mysqli_free_result($result);


	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_047_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_047_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>