<?php
	require_once("connect.inc");
	$test_table_name = 'test_mysqli_get_server_info_table_1'; require "table.inc";
	if (!is_string($info = mysqli_get_server_info($link)) || ('' === $info))
		printf("[003] Expecting string/any_non_empty, got %s/%s\n", gettype($info), $info);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_get_server_info_table_1'; require_once("clean_table.inc");
?>