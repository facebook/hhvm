<?php
	include "connect.inc";

	$tmp    = NULL;
	$link   = NULL;

 	if ($socket)
        $host = sprintf("%s:%s", $host, $socket);
    else if ($port)
        $host = sprintf("%s:%s", $host, $port);

	function connect($host, $user, $passwd) {
		$conn = mysql_pconnect($host, $user, $passwd);

		if (!$conn)
			die(sprintf("[001] %s\n", mysql_error()));

		if (!mysql_query("set wait_timeout=1", $conn))
			printf("[002] [%d] %s\n", mysql_errno($conn), mysql_error($conn));

		return $conn;
	}	

	$conn = connect($host, $user, $passwd);
	$opened_files = -1;

	for ($i = 0; $i < 4; $i++) {
		/* wait while mysql closes connection */
		sleep(3);

		if (!mysql_ping($conn)) {
			printf("[003] reconnect %d\n", $i);
			$conn = connect($host, $user, $passwd);  
		}

		$r = mysql_query('select 1', $conn);
		if (!$r)
			printf("[004] [%d] %s\n", mysql_errno($conn), mysql_error($conn));


		if ($opened_files == -1) {
			$opened_files = trim(exec("lsof -np " . getmypid() . " | wc -l"));
			printf("[005] Setting openened files...\n");
		} else if (($tmp = trim(exec("lsof -np " . getmypid() . " | wc -l"))) != $opened_files) {
			printf("[006] [%d] different number of opened_files : expected %d, got %d", $i, $opened_files, $tmp);
		} else {
			printf("[007] Opened files as expected\n");
		}
	}

	print "done!";
?>
