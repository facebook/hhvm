<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!mysqli_query($link, "DROP TABLE IF EXISTS test_005_table_1"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, "CREATE TABLE test_005_table_1(c1 char(10), c2 text) ENGINE=" . $engine))
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$a = str_repeat("A1", 32000);

	mysqli_query($link, "INSERT INTO test_005_table_1 VALUES ('1234567890', '$a')");

	$stmt = mysqli_prepare($link, "SELECT * FROM test_005_table_1");
	mysqli_stmt_bind_result($stmt, $c1, $c2);
	mysqli_stmt_execute($stmt);
	mysqli_stmt_fetch($stmt);

	$test[] = $c1;
	$test[] = ($a == $c2) ? "32K String ok" : "32K String failed";

	var_dump($test);

	/* this will crash with libmysql from PHP 5.0.6 (or earlier) to 5.3.0 */
	mysqli_stmt_fetch($stmt);

	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_005_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_005_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>