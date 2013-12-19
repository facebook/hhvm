<?php
	set_time_limit(12);
	include ("connect.inc");

	if (!$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysql_errno(), mysqlerror());
	}

	if (!$res = mysql_query("SELECT SLEEP(6)", $link))
		printf("[002] [%d] %s\n",  mysql_errno($link), mysql_error($link));

	var_dump(mysql_fetch_assoc($res));

	mysql_free_result($res);
	mysql_close($link);

	print "done!";
?>