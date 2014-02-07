<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("DROP TABLE IF EXISTS test_066_table_1");

	$mysql->query("CREATE TABLE test_066_table_1 (a int not null) ENGINE=myisam");

	$mysql->query("INSERT INTO test_066_table_1 VALUES (1),(2),(NULL)");

	if (($warning = $mysql->get_warnings())) {
		do {
			printf("Warning\n");
		} while ($warning->next());
	}

	$mysql->close();
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_066_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>