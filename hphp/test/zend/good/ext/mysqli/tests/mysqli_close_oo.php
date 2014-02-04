<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	if (!$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	$tmp = $mysqli->close();
	if (true !== $tmp)
		printf("[003] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);
	print "done!";
?>