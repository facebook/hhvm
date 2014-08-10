<?php
	/* NOTE: tests is a stub, but function is deprecated, as long as it does not crash when invoking it... */
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	}

	$query = array();
	if (!is_int($tmp = mysqli_send_query($link, 'SELECT 1')))
		printf("[005] Expecting integer/any value, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_send_query($link, 'SELECT 1')))
		printf("[006] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>