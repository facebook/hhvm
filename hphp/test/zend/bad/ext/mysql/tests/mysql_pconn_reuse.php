<?php
	require_once("connect.inc");
	require_once("table.inc");
	mysql_close($link);

	if ($socket)
		$myhost = sprintf("%s:%s", $host, $socket);
	else if ($port)
		$myhost = sprintf("%s:%s", $host, $port);
	else
	$myhost = $host;

	if (($plink = mysql_pconnect($myhost, $user, $passwd)))
		printf("[001] Can connect to the server.\n");

	if ((mysql_select_db($db, $plink)) &&
			($res = mysql_query('SELECT id FROM test', $plink)) &&
			($row = mysql_fetch_assoc($res)) &&
			(mysql_free_result($res))) {
		printf("[002] Can fetch data using persistent connection! Data = '%s'\n",
			$row['id']);
	} else {
		printf("[002] [%d] %s\n", mysql_errno($plink), mysql_error($plink));
	}

	$thread_id = mysql_thread_id($plink);
	mysql_close($plink);

	if (!($plink = mysql_pconnect($myhost, $user, $passwd)))
		printf("[003] Cannot connect, [%d] %s\n", mysql_errno(), mysql_error());

	if (mysql_thread_id($plink) != $thread_id)
		printf("[004] Looks like the second call to pconnect() did not give us the same connection.\n");

	$thread_id = mysql_thread_id($plink);
	mysql_close($plink);

	if (!($plink = mysql_connect($myhost, $user, $passwd, true)))
		printf("[005] Cannot connect, [%d] %s\n", mysql_errno(), mysql_error());

	if (mysql_thread_id($plink) == $thread_id)
		printf("[006] Looks like connect() did not return a new connection.\n");

	if (($link = mysql_connect($myhost, $user, $passwd, true)))
		printf("[007] Can connect although limit has been reached, [%d] %s\n", mysql_errno(), mysql_error());

	print "done!";
?>