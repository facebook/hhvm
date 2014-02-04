<?php
	require_once("connect.inc");

	$strict_on = false;
	if (defined('E_STRICT')) {
		error_reporting(((int)ini_get('error_reporting')) | E_STRICT );
		$strict_on = true;
	}

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_more_results_table_1'; require('table.inc');

	print "[004]\n";
	var_dump(mysqli_more_results($link));

	if (!mysqli_multi_query($link, "SELECT 1 AS a; SELECT 1 AS a, 2 AS b; SELECT id FROM test_mysqli_more_results_table_1 ORDER BY id LIMIT 3"))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	print "[006]\n";
	$i = 1;

	if ($strict_on)
		ob_start();

	if (mysqli_get_server_version($link) > 41000 && !($ret = mysqli_more_results($link)))
		printf("[007] Expecting boolean/true, got %s/%s\n", gettype($ret), $ret);
	do {
		$res = mysqli_store_result($link);
		mysqli_free_result($res);
		if (mysqli_more_results($link))
			printf("%d\n", $i++);
	} while (mysqli_next_result($link));

	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[008] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
			print trim($tmp) . "\n";
	}

	if (!mysqli_multi_query($link, "SELECT 1 AS a; SELECT 1 AS a, 2 AS b; SELECT id FROM test_mysqli_more_results_table_1 ORDER BY id LIMIT 3"))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	print "[010]\n";
	$i = 1;
	if (mysqli_get_server_version($link) > 41000 && !($ret = mysqli_more_results($link)))
		printf("[011] Expecting boolean/true, got %s/%s\n", gettype($ret), $ret);

	if ($strict_on)
		ob_start();
	do {
		$res = mysqli_use_result($link);
		// NOTE: if you use mysqli_use_result() with mysqli_more_results() or any other info function,
		// you must fetch all rows before you can loop to the next result set!
		// See also the MySQL Reference Manual: mysql_use_result()
		while ($row = mysqli_fetch_array($res))
			;
		mysqli_free_result($res);
		if (mysqli_more_results($link))
			printf("%d\n", $i++);
	} while (mysqli_next_result($link));

	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[008] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
		print trim($tmp) . "\n";
	}
	mysqli_close($link);

	var_dump(mysqli_more_results($link));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_more_results_table_1'; require_once("clean_table.inc");
?>