<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_ping_table_1'; require('table.inc');
	var_dump(mysqli_ping($link));

	// provoke an error to check if mysqli_ping resets it
	$res = mysqli_query($link, 'SELECT * FROM unknown_table');
	if (!($errno = mysqli_errno($link)))
		printf("[003] Statement should have caused an error\n");

	var_dump(mysqli_ping($link));
	if ($errno === mysqli_errno($link))
		printf("[004] Error codes should have been reset\n");

	mysqli_close($link);

	if (!is_null($tmp = mysqli_ping($link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
