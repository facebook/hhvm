<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_debug_control_string_table_1'; require_once('table.inc');

	function try_control_string($link, $control_string, $trace_file, $offset) {

		if (true !== ($tmp = mysqli_debug($control_string))) {
			printf("[%03d][control string '%s'] Expecting boolean/true, got %s/%s.\n",
				$offset + 1,
				$control_string,
				gettype($tmp),
				$tmp);
			return false;
		}

		if (!$res = mysqli_query($link, 'SELECT * FROM test')) {
			printf("[%03d][control string '%s'] [%d] %s.\n",
				$offset + 2,
				$control_string,
				mysqli_errno($link),
				mysqli_error($link));
			return false;
		}

		clearstatcache();
		if (!file_exists($trace_file)) {
			printf("[%03d][control string '%s'] Trace file has not been written.\n",
				$offset + 3,
				$control_string,
				gettype($tmp),
				$tmp);
			return false;
		}

		unlink($trace_file);
	}

	$trace_file = sprintf('%s%s%s', sys_get_temp_dir(), DIRECTORY_SEPARATOR, 'mysqli_debug_phpt.trace');
	try_control_string($link, 't:,,:o,' . $trace_file, $trace_file, 10);
	try_control_string($link, ':' . chr(0) . 'A,' . $trace_file, $trace_file, 20);
	try_control_string($link, 't:o,' . $trace_file . ':abc', $trace_file, 30);
	try_control_string($link, 't:o,' . $trace_file . ':ABC,123:b', $trace_file, 40);
	try_control_string($link, 't:ABC,123:b;:o,' . $trace_file, $trace_file, 50);
	try_control_string($link, 't:A;BC,123:b;:o,' . $trace_file, $trace_file, 60);
	try_control_string($link, 't:p:o,' . $trace_file, $trace_file, 70);
	try_control_string($link, 't:p,1,,2:o,' . $trace_file, $trace_file, 80);
	try_control_string($link, 't:z,1,,2:o,' . $trace_file, $trace_file, 90);#

	mysqli_close($link);
	print "done";
	if ($IS_MYSQLND)
		print "libmysql/DBUG package prints some debug info here."
?>