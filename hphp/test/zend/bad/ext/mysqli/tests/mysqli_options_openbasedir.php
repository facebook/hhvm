<?php
	require_once('connect.inc');
	ini_set("open_basedir", __DIR__);
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[001] Cannot connect, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

	if (false !== mysqli_options($link, MYSQLI_OPT_LOCAL_INFILE, 1))
		printf("[002] Can set MYSQLI_OPT_LOCAL_INFILE although open_basedir is set!\n");

	mysqli_close($link);
	print "done!";
?>
