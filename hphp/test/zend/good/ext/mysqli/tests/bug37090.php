<?php
	require_once("connect.inc");

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	$cs = array();
	$cs[] = $mysql->set_charset("latin1");
	$cs[] = $mysql->character_set_name();

	$cs[] = $mysql->set_charset("utf8");
	$cs[] = $mysql->character_set_name();

	$cs[] = $mysql->set_charset("notdefined");
	$cs[] = $mysql->character_set_name();

	var_dump($cs);
	print "done!";
?>