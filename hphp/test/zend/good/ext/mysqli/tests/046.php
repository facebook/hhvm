<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link, "DROP TABLE IF EXISTS test_046_table_1");
	mysqli_query($link, "CREATE TABLE test_046_table_1 (foo int) ENGINE=" . $engine);

	mysqli_query($link, "INSERT INTO test_046_table_1 VALUES (1),(2),(3),(4),(5)");

	$stmt = mysqli_prepare($link, "DELETE FROM test_046_table_1 WHERE foo=?");
	mysqli_stmt_bind_param($stmt, "i", $c1);

	$c1 = 2;

	mysqli_stmt_execute($stmt);
	$x = mysqli_stmt_affected_rows($stmt);

	mysqli_stmt_close($stmt);
	var_dump($x==1);

	mysqli_query($link, "DROP TABLE IF EXISTS test_046_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_046_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>