<?php
require_once('connect.inc');

function my_mysql_pconnect($host, $user, $passwd, $db, $port, $socket) {
	if ($socket)
		$host = sprintf("%s:%s", $host, $socket);
	else if ($port)
		$host = sprintf("%s:%s", $host, $port);

	if (!$link = mysql_pconnect($host, $user, $passwd, true)) {
		printf("[000-a] Cannot connect using host '%s', user '%s', password '****', [%d] %s\n",
			$host, $user, $passwd,
			mysql_errno(), mysql_error());
		return false;
	}
	return $link;
}

echo "Explicit connection on close\n";
$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket);
$link1_thread_id = mysql_thread_id($link);
$default1_thread_id = mysql_thread_id();
echo 'Expect same thread id for $link and default conn: ';
var_dump($link1_thread_id == $default1_thread_id);
var_dump($link);
mysql_close($link);
var_dump($link);

// we sohuld have no default link anymore
mysql_close();

echo "\nClosing default link\n";
$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket);
$link2_thread_id = mysql_thread_id($link);
$default2_thread_id = mysql_thread_id();
echo 'Expect same thread id for $link and default conn but not the previous: ';
var_dump($link1_thread_id == $default1_thread_id && $link1_thread_id != $link2_thread_id);
var_dump($link);
mysql_close();
var_dump($link);
mysql_close($link);
var_dump($link);

echo "\nExplicit resource and pconnect\n";
$link = my_mysql_pconnect($host, $user, $passwd, $db, $port, $socket);
var_dump($link);
mysql_close($link);
var_dump($link);

// we sohuld have no default link
mysql_close();

echo "\nDefault link and pconnect\n";
$link = my_mysql_pconnect($host, $user, $passwd, $db, $port, $socket);
var_dump($link);
mysql_close();
var_dump($link);
mysql_close($link);
var_dump($link);
?>