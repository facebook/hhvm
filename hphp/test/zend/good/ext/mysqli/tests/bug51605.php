<?php
	include ("connect.inc");

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[002] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}
	mysqli_close($link);
	echo "closed once\n";

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[002] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}
	mysqli_close($link);
	echo "closed twice\n";

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}
	mysqli_close($link);
	echo "closed for third time\n";

	print "done!";
?>