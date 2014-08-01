<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	mysqli_select_db($link, $db);

	mysqli_query($link, "DROP TABLE IF EXISTS test_036_table_1");
	mysqli_query($link, "CREATE TABLE test_036_table_1 (a bigint not null auto_increment primary key, b varchar(10)) ENGINE = " . $engine);

	mysqli_query($link, "INSERT INTO test_036_table_1 (b) VALUES ('foo1')");
	$test[] = mysqli_insert_id($link);

	/* we have to insert more values, cause lexer sets auto_increment to max_int
	   see mysql bug #54. So we don't check for the value, only for type (which must
	   be type string)
	*/

	mysqli_query($link, "ALTER TABLE test_036_table_1 AUTO_INCREMENT=9999999999999998");
	mysqli_query($link, "INSERT INTO test_036_table_1 (b) VALUES ('foo2')");
	mysqli_query($link, "INSERT INTO test_036_table_1 (b) VALUES ('foo3')");
	mysqli_query($link, "INSERT INTO test_036_table_1 (b) VALUES ('foo4')");
	$x = mysqli_insert_id($link);
	$test[] = is_string($x);

	var_dump($test);

	mysqli_query($link, "DROP TABLE IF EXISTS test_036_table_1");
	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_036_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>