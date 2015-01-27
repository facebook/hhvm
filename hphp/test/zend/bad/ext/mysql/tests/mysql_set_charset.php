<?php
include_once "connect.inc";

$tmp    = NULL;
$link   = NULL;

if (!is_null($tmp = @mysql_set_charset()))
	printf("[001] Expecting NULL got %s/%s\n", $tmp, gettype($tmp));

if (false !== ($tmp = @mysql_set_charset($link)))
	printf("[002] Expecting boolean/false got %s/%s\n", $tmp, gettype($tmp));

if (false !== ($tmp = @mysql_set_charset(-1)))
	printf("[003] Expecting boolean/false got %s/%s\n", $tmp, gettype($tmp));

if (!is_null($tmp = @mysql_set_charset('somecharset', $link)))
	printf("[004] Expecting NULL got %s/%s\n", $tmp, gettype($tmp));

if (!$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket))
	printf("[005] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);

/* unicode mode should throw a warning */
$tmp = mysql_set_charset('uFt8', $link);

if ((version_compare(PHP_VERSION, '5.9.9', '>') == 1))
	$expect = false;
else
	$expect = true;

$charsets = array('latin1', 'latin2');
foreach ($charsets as $k => $charset) {
	if (!($res = mysql_query(sprintf('SHOW CHARACTER SET LIKE "%s"', $charset), $link)))
		continue;
	mysql_free_result($res);
	if ($expect !== ($tmp = @mysql_set_charset($charset, $link)))
		printf("[006] Expecting %s/%s got %s/%s\n",
			gettype($expect), $expect,
			gettype($tmp), $tmp);
}

mysql_close($link);
print "done!";
?>
