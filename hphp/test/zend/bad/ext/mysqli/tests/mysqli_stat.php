<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	if (!is_null($tmp = @mysqli_stat()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_stat($link)))
		printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$test_table_name = 'test_mysqli_stat_table_1'; require('table.inc');

	if (!is_null($tmp = @mysqli_stat($link, "foo")))
		printf("[003] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if ((!is_string($tmp = mysqli_stat($link))) || ('' === $tmp))
		printf("[004] Expecting non empty string, got %s/'%s', [%d] %s\n",
			gettype($tmp), $tmp, mysqli_errno($link), mysql_error($link));

	mysqli_close($link);

	if (!is_null($tmp = mysqli_stat($link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>