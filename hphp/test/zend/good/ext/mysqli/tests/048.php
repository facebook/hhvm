<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$mysql = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$mysql->select_db($db);
	$mysql->query("DROP TABLE IF EXISTS test_048_table_1");

	$mysql->query("CREATE TABLE test_048_table_1(col1 tinyint, col2 smallint,
		col3 int, col4 bigint,
		col5 float, col6 double,
		col7 date, col8 time,
		col9 varbinary(10),
		col10 varchar(50),
		col11 char(20)) ENGINE=" . $engine);

	$mysql->query("INSERT INTO test_048_table_1(col1,col10, col11) VALUES(1,'foo1', 1000),(2,'foo2', 88),(3,'foo3', 389789)");

	$stmt = $mysql->prepare("SELECT col1, col2, col3, col4, col5, col6, col7, col8, col9, col10, col11 from test_048_table_1");
	$stmt->bind_result($c1, $c2, $c3, $c4, $c5, $c6, $c7, $c8, $c9, $c10, $c11);
	$stmt->execute();

	$stmt->fetch();

	$test = array($c1,$c2,$c3,$c4,$c5,$c6,$c7,$c8,$c9,$c10,$c11);

	var_dump($test);

	$stmt->close();
	$mysql->query("DROP TABLE IF EXISTS test_048_table_1");
	$mysql->close();
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_048_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>