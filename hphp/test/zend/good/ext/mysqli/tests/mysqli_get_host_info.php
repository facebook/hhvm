<?php
	require_once("connect.inc");
	$test_table_name = 'test_mysqli_get_host_info_table_1'; require "table.inc";
	if (!is_string($info = mysqli_get_host_info($link)) || ('' === $info))
		printf("[003] Expecting string/any_non_empty, got %s/%s\n", gettype($info), $info);

	if ($IS_MYSQLND && $host != 'localhost' && $host != '127.0.0.1' && $port != '' && $host != "" && strtoupper(substr(PHP_OS, 0, 3)) != 'WIN') {
		/* this should be a TCP/IP connection and not a Unix Socket (or SHM or Named Pipe) */
		if (!stristr($info, "TCP/IP"))
			printf("[004] Should be a TCP/IP connection but mysqlnd says '%s'\n", $info);	
	}
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_get_host_info_table_1'; require_once("clean_table.inc");
?>