<?php
require_once('connect.inc');

function my_connect($offset, $host, $user, $passwd, $db, $port, $socket) {
	if ($socket)
		$host = sprintf("%s:%s", $host, $socket);
	else if ($port)
		$host = sprintf("%s:%s", $host, $port);

	$link = mysql_connect($host, $user, $passwd, true);

	if (!$link) {
		printf("[%03d] Cannot connect using host '%s', user '%s', password '****', [%d] %s\n",
			$offset, $host, $user, $passwd,
			mysql_errno(), mysql_error());
		return false;
	}

	return $link;
}

$links = array();

// try to open 3 links
$links[0] = my_connect(10, $host, $user, $passwd, $db, $port, $socket);
$links[1] = my_connect(20, $host, $user, $passwd, $db, $port, $socket);
$links[2] = my_connect(30, $host, $user, $passwd, $db, $port, $socket);
if (false !== $links[2])
	printf("[040] Last connection should not have been allowed!\n");

// free some links but let index 1 remain
unset($links[2]);
mysql_close($links[0]);
unset($links[0]);

// should be allowed -> second open connection
$links[0] = my_connect(50, $host, $user, $passwd, $db, $port, $socket);
$links[2] = my_connect(60, $host, $user, $passwd, $db, $port, $socket);
ksort($links);
var_dump($links);

mysql_close($links[0]);
mysql_close($links[1]);
print "done!\n";
?>