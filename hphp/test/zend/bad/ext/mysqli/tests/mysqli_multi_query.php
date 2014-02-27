<?php
	require_once("connect.inc");

	$strict_on = false;
	if (defined('E_STRICT')) {
		error_reporting(((int)ini_get('error_reporting')) | E_STRICT );
		$strict_on = true;
	}
	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_multi_query_table_1'; require('table.inc');

	if (false !== ($tmp = mysqli_multi_query($link, "")))
		printf("[003] Expecting boolean/false, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_multi_query($link, "SELECT 1 AS a; SELECT 1 AS a, 2 AS b; SELECT id FROM test_mysqli_multi_query_table_1 ORDER BY id LIMIT 3"))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if ($strict_on)
		ob_start();

	$i = 0;
	do {
		$res = mysqli_store_result($link);
		while ($row = mysqli_fetch_array($res))
			;
		mysqli_free_result($res);
		$i++;
	} while (mysqli_next_result($link));

	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[005a] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
		print trim($tmp) . "\n";
	}

	printf("[006] %d\n", $i);

	if (!mysqli_multi_query($link, "ALTER TABLE test_mysqli_multi_query_table_1 MODIFY id INT AUTO_INCREMENT; INSERT INTO test_mysqli_multi_query_table_1(label) VALUES ('a'); SELECT id, label FROM test_mysqli_multi_query_table_1 ORDER BY id"))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$i = 0;
	while (mysqli_next_result($link) && ($res = mysqli_store_result($link))) {

		while ($row = mysqli_fetch_array($res))
			;
		mysqli_free_result($res);
		printf("%d/%d\n", $i, mysqli_insert_id($link));
		$i++;
	}
	printf("[008] %d\n", $i);

	if (!mysqli_multi_query($link, "SELECT id, label FROM test_mysqli_multi_query_table_1"))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if ($strict_on)
		ob_start();
	$i = 0;
	while (mysqli_next_result($link) && ($res = mysqli_store_result($link))) {
		while ($row = mysqli_fetch_array($res))
			$i++;
		mysqli_free_result($res);
	}
	if ($strict_on) {
		$tmp = ob_get_contents();
		ob_end_clean();
		if (!preg_match('@Strict Standards: mysqli_next_result\(\): There is no next result set@ismU', $tmp)) {
			printf("[009a] Strict Standards warning missing\n");
		} else {
			$tmp = trim(preg_replace('@Strict Standards: mysqli_next_result\(\).*on line \d+@ism', '', $tmp));
		}
		print trim($tmp) . "\n";
	}
	printf("[010] %d\n", $i);

	if (!mysqli_multi_query($link, "SELECT 1 AS num, 'a' AS somechar; SELECT 2 AS num, 'a' AS somechar; SELECT 3 AS num, 'a' AS somechar"))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$res_num = 1;
	do {
		if (!$res = mysqli_store_result($link)) {
			printf("[012 - %d] [%d] %s\n", $res_num, mysqli_errno($link), mysqli_error($link));
			continue;
		}

		$num_rows = 0;
		while ($row = mysqli_fetch_array($res)) {

			$num_rows++;
			if ($row['num'] != $res_num)
				printf("[013 - %d] Expecting %s got %s\n", $res_num, $res_num, $row['num']);
			if ($row['somechar'] != "a")
				printf("[014 - %d] Expecting a got %s\n", $res_num, $row['somechar']);

			if (1 == $num_rows) {
				/* simple metadata check */
				if (!($lengths = mysqli_fetch_lengths($res)))
					printf("[015 - %d] [%d] %s\n", $res_num, mysqli_errno($link), mysqli_error($link));

				if (count($lengths) != 2)
					printf("[016 - %d] Expecting 2 column lengths got %d [%d] %s\n", $res_num, count($lengths));

				foreach ($lengths as $k => $length)
					if ($length <= 0)
						printf("[017 - %d] Strange column lengths for column %d, got %d expecting any > 0\n",
							$res_num, $k, $length);
				}
		}

		if ($num_rows != 1)
			printf("[018 - %d] Expecting 1 row, got %d rows\n", $num_rows);

		$res_num++;

		mysqli_free_result($res);

	} while (@mysqli_next_result($link));

	if ($res_num != 4)
		printf("[015] Expecting 3 result sets got %d result set[s]\n", $res_num);

	mysqli_close($link);

	var_dump(mysqli_multi_query($link, "SELECT id, label FROM test_mysqli_multi_query_table_1"));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_multi_query_table_1'; require_once("clean_table.inc");
?>