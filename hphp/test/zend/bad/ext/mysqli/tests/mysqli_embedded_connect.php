<?php
	require_once("connect.inc");
	$tmp    = NULL;
	$link   = NULL;
	if (!$link = mysqli_embedded_connect($db)) {
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	}

	if (!is_bool($tmp = mysqli_embedded_connect($db . '_unknown')))
		printf("[003] Expecting boolean/[true|false] value, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	print "done!";
?>