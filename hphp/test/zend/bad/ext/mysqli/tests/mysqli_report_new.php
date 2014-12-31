<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$test_table_name = 'test_mysqli_report_new_table_1'; require('table.inc');

	/*
	Internal macro MYSQL_REPORT_ERROR
	*/
	mysqli_report(MYSQLI_REPORT_ERROR);

	mysqli_change_user($link, "0123456789-10-456789-20-456789-30-456789-40-456789-50-456789-60-456789-70-456789-80-456789-90-456789", "password", $db);

	mysqli_report(MYSQLI_REPORT_OFF);

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
	  printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);

	mysqli_change_user($link, "This might work if you accept anonymous users in your setup", "password", $db);

	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_report_new_table_1'; require_once("clean_table.inc");
?>