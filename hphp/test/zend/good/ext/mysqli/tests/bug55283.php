<?php
	include "connect.inc";
	$db1 = new mysqli();


	$flags = MYSQLI_CLIENT_SSL;

	$link = mysqli_init();
	mysqli_ssl_set($link, null, null, null, null, "RC4-MD5");
	if (my_mysqli_real_connect($link, 'p:' . $host, $user, $passwd, $db, $port, null, $flags)) {
		$r = $link->query("SHOW STATUS LIKE 'Ssl_cipher'");
		var_dump($r->fetch_row());
	}

	/* non-persistent connection */
	$link2 = mysqli_init();
	mysqli_ssl_set($link2, null, null, null, null, "RC4-MD5");
	if (my_mysqli_real_connect($link2, $host, $user, $passwd, $db, $port, null, $flags)) {
		$r2 = $link2->query("SHOW STATUS LIKE 'Ssl_cipher'");
		var_dump($r2->fetch_row());
	}

	echo "done\n";
?>