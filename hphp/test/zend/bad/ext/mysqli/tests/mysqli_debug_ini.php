<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_debug_ini_table_1'; require_once('table.inc');

	var_dump(ini_get('mysqlnd.debug'));

	$trace_file = '/tmp/mysqli_debug_phpt.trace';
	clearstatcache();
	if (!file_exists($trace_file))
		printf("[003] Trace file '%s' has not been created\n", $trace_file);
	if (filesize($trace_file) < 50)
		printf("[004] Trace file '%s' is very small. filesize() reports only %d bytes. Please check.\n",
			$trace_file,
			filesize($trace_file));

	mysqli_close($link);
	unlink($trace_file);

	print "done!";
?>