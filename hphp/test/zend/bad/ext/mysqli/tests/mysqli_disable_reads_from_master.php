<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	}

	if (!is_bool($tmp = mysqli_disable_reads_from_master($link)))
		printf("[004] Expecting boolean/[true|false] value, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_disable_reads_from_master($link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_disable_reads_from_master_table_1'; require_once("clean_table.inc");
?>