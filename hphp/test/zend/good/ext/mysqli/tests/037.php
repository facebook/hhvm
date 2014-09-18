<?php

	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link, "DROP TABLE IF EXISTS test_037_table_1");

	mysqli_query($link, "CREATE TABLE test_037_table_1 (a int, b varchar(10)) ENGINE = " . $engine);

	mysqli_query($link, "INSERT INTO test_037_table_1 VALUES (1, 'foo')");
	$ir[] = mysqli_field_count($link);

	mysqli_real_query($link, "SELECT * FROM test_037_table_1");
	$ir[] = mysqli_field_count($link);


	var_dump($ir);

	mysqli_query($link, "DROP TABLE IF EXISTS test_037_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_037_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>