<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$mysql = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("DROP TABLE IF EXISTS test_bug36745_table_1");
	$mysql->query("CREATE TABLE test_bug36745_table_1 (a VARCHAR(20))");

	$mysql->query("LOAD DATA LOCAL INFILE 'filenotfound' INTO TABLE test_bug36745_table_1");
	var_dump($mysql->error);

	$mysql->close();
	printf("Done");
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_bug36745_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>