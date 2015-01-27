<?php
	require_once("connect.inc");
	$test_table_name = 'test_mysqli_max_links_table_1'; require_once("table.inc");

	// to make sure we have at least one working connection...
	var_dump(mysqli_ping($link));
	// to make sure that max_links is really set to one
	var_dump((int)ini_get('mysqli.max_links'));

	$links = array();
	for ($i = 1; $i <= 5; $i++)
		if ($links[$i] = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
			printf("[%03d] One link is already open, it should not be possible to open more, [%d] %s, [%d] %s\n",
				$i, mysqli_connect_errno(), mysqli_connect_error(),
				mysqli_errno($links[$i]), mysqli_error($links[$i]));

	for ($i = 1; $i <= 5; $i++) {
		if ($res = mysqli_query($links[$i], 'SELECT id FROM test_mysqli_max_links_table_1 LIMIT 1')) {
			printf("[%03d] Can run query on link %d\n", 5 + $i, $i);
			mysqli_free_result($res);
		}
		mysqli_close($links[$i]);
	}

	mysqli_close($link);
	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_max_links_table_1'; require_once("clean_table.inc");
?>