<?php
	require_once("connect.inc");

	$tmp	= NULL;
	$link	= NULL;

	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n", $host, $user, $db, $port, $socket);
	if (!is_bool($tmp = $mysqli->dump_debug_info()))
		printf("[003] Expecting boolean/[true|false] value, got %s/%s, [%d] %s\n",
			gettype($tmp), $tmp,
			$mysqli->errno, $mysqli->error);

	$mysqli->close();

	if (NULL !== ($tmp = $mysqli->dump_debug_info()))
		printf("[004] Expecting NULL, got %s/%s, [%d] %s\n",
			gettype($tmp), $tmp,
			$mysqli->errno, $mysqli->error);

	print "done!";
?>
