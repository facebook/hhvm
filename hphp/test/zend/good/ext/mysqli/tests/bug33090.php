<?php
	include ("connect.inc");

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, null, $port, $socket);
	mysqli_select_db($link, $db);

	if (!($link->prepare("this makes no sense"))) {
		printf("%d\n", $link->errno);
		printf("%s\n", $link->sqlstate);
	}
	$link->close();
?>