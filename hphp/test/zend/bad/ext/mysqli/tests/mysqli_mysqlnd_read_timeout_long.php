<?php
	set_time_limit(12);
	include ("connect.inc");

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!$res = mysqli_query($link, "SELECT SLEEP(6)"))
		printf("[002] [%d] %s\n",  mysqli_errno($link), mysqli_error($link));

	var_dump($res->fetch_assoc());

	mysqli_free_result($res);
	mysqli_close($link);

	print "done!";
?>