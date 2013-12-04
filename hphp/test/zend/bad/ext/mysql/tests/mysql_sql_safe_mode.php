<?php
require_once('connect.inc');
if ($socket)
	$host = sprintf("%s:%s", $host, $socket);
else if ($port)
	$host = sprintf("%s:%s", $host, $port);

if ($link = mysql_connect($host, $user, $passwd, true)) {
	printf("[001] Safe mode not working properly?\n");
	mysql_close($link);
}

if ($link = mysql_pconnect($host, $user, $passwd)) {
	printf("[002] Safe mode not working properly?\n");
	mysql_close($link);
}
print "done!\n";
?>