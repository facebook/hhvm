<?php
	require_once("connect.inc");
	$test_table_name = 'test_mysqli_info_table_1'; require "table.inc";
	if (!$res = mysqli_query($link, "INSERT INTO test_mysqli_info_table_1(id, label) VALUES (100, 'a')"))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	// NOTE: empty string, no multiple insert syntax
	if (!is_null($tmp = mysqli_info($link)) || ('' != $tmp))
		printf("[004] Expecting null, got %s/%s\n", gettype($tmp), $tmp);

	if (!$res = mysqli_query($link, "INSERT INTO test_mysqli_info_table_1(id, label) VALUES (101, 'a'), (102, 'b')"))
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_string($tmp = mysqli_info($link)) || ('' == $tmp))
		printf("[006] Expecting string/any_non_empty, got %s/%s\n", gettype($tmp), $tmp);

	if ((version_compare(PHP_VERSION, '5.9.9', '>') == 1) && !is_unicode($tmp))
		printf("[007] Expecting unicode, because unicode mode it on. Got binary string\n");

	if (!$res = mysqli_query($link, 'INSERT INTO test_mysqli_info_table_1(id, label) SELECT id + 200, label FROM test_mysqli_info_table_1'))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_string($tmp = mysqli_info($link)) || ('' == $tmp))
		printf("[008] Expecting string/any_non_empty, got %s/%s\n", gettype($tmp), $tmp);

	if (!$res = mysqli_query($link, 'ALTER TABLE test_mysqli_info_table_1 MODIFY label CHAR(2)'))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_string($tmp = mysqli_info($link)) || ('' == $tmp))
		printf("[010] Expecting string/any_non_empty, got %s/%s\n", gettype($tmp), $tmp);

	if (!$res = mysqli_query($link, "UPDATE test_mysqli_info_table_1 SET label = 'b' WHERE id >= 100"))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_string($tmp = mysqli_info($link)) || ('' == $tmp))
		printf("[012] Expecting string/any_non_empty, got %s/%s\n", gettype($tmp), $tmp);

	if (!$res = mysqli_query($link, "SELECT 1"))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!is_null($tmp = mysqli_info($link)) || ('' != $tmp))
		printf("[014] Expecting null, got %s/%s\n", gettype($tmp), $tmp);
	mysqli_free_result($res);

	// NOTE: no LOAD DATA INFILE test_mysqli_info_table_1
	if ($dir = sys_get_temp_dir()) {
		do {
			$file = $dir . '/' . 'mysqli_info_phpt.cvs';
			if (!$fp = fopen($file, 'w'))
				/* ignore this error */
				break;

			if (!fwrite($fp, b"100;'a';\n") ||
				!fwrite($fp, b"101;'b';\n") ||
				!fwrite($fp, b"102;'c';\n")) {
				@unlink($file);
				break;
			}
			fclose($fp);
			if (!mysqli_query($link, "DELETE FROM test_mysqli_info_table_1")) {
				printf("[015] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
				break;
			}

			if (!@mysqli_query($link, sprintf("LOAD DATA LOCAL INFILE '%s' INTO TABLE test_mysqli_info_table_1 FIELDS TERMINATED BY ';' OPTIONALLY ENCLOSED BY '\'' LINES TERMINATED BY '\n'", $file))) {
				/* ok, because we might not be allowed to do this */
				@unlink($file);
				break;
			}

			if (!is_string($tmp = mysqli_info($link)) || ('' == $tmp))
				printf("[016] Expecting string/any_non_empty, got %s/%s\n", gettype($tmp), $tmp);

			unlink($file);
		} while (false);
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_info_table_1'; require_once("clean_table.inc");
?>