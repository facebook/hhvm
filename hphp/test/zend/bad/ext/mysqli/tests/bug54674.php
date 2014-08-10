<?php
	include ("connect.inc");

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[002] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	$japanese_so = pack('H4', '835c');
	$link->set_charset('sjis');
	var_dump($link->real_escape_string($japanese_so) === $japanese_so);
	mysqli_close($link);

	print "done!";
?>