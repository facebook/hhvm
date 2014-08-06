<?php
	class really_my_mysqli extends mysqli {
		function __construct()
		{
		}
	}

	require_once("connect.inc");
	$mysql = mysqli_init();

	/* following operations should not work */
	if (method_exists($mysql, 'set_charset')) {
		$x[0] = @$mysql->set_charset('utf8');
	} else {
		$x[0] = NULL;
	}
	$x[1] = @$mysql->query("SELECT 'foo' FROM test_bug36802_table_1");

	/* following operations should work */
	$x[2] = ($mysql->client_version > 0);
	$x[3] = $mysql->errno;
	$mysql->close();

	var_dump($x);
?>
