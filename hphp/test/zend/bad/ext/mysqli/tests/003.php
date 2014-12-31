<?php
	require_once("connect.inc");

	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_query($link, "SET sql_mode=''");

	if (!mysqli_query($link,"DROP TABLE IF EXISTS test_003_table_1"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$rc = @mysqli_query($link,"CREATE TABLE test_003_table_1(
		c1 date,
		c2 time,
		c3 timestamp(14),
		c4 year,
		c5 datetime,
		c6 timestamp(4),
		c7 timestamp(6)) ENGINE=" . $engine);

	/*
	Seems that not all MySQL 6.0 installations use defaults that ignore the display widths.
	From the manual:
	From MySQL 4.1.0 on, TIMESTAMP display format differs from that of earlier MySQL releases:
	[...]
	Display widths (used as described in the preceding section) are no longer supported.
	In other words, for declarations such as TIMESTAMP(2), TIMESTAMP(4), and so on,
	the display width is ignored.
	[...]
	*/
	if (!$rc)
		$rc = @mysqli_query($link,"CREATE TABLE test_003_table_1(
			c1 date,
			c2 time,
			c3 timestamp,
			c4 year,
			c5 datetime,
			c6 timestamp,
			c7 timestamp) ENGINE=" . $engine);

	if (!$rc)
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$rc = mysqli_query($link, "INSERT INTO test_003_table_1 VALUES(
		'2002-01-02',
		'12:49:00',
		'2002-01-02 17:46:59',
		2010,
		'2010-07-10',
		'2020','1999-12-29')");
	if (!$rc)
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$stmt = mysqli_prepare($link, "SELECT c1, c2, c3, c4, c5, c6, c7 FROM test_003_table_1");
	mysqli_stmt_bind_result($stmt, $c1, $c2, $c3, $c4, $c5, $c6, $c7);
	mysqli_stmt_execute($stmt);
	mysqli_stmt_fetch($stmt);

	$test = array($c1,$c2,$c3,$c4,$c5,$c6,$c7);

	var_dump($test);

	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_003_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php error_reporting(0); ?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_003_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>