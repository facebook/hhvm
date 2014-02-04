<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$sinfo = substr(mysqli_get_server_info($link),0,1);

	var_dump(strlen($sinfo));

	mysqli_close($link);
	print "done!";
?>