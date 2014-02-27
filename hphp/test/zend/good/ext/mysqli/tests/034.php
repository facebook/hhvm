<?php
	require_once("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$pinfo = mysqli_get_proto_info($link);

	var_dump($pinfo);

	mysqli_close($link);
	print "done!";
?>