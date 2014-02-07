<?php
	$test_table_name = 'test_mysqli_fetch_assoc_no_alias_table_1'; require('table.inc');

	if (!$res = mysqli_query($link, "SELECT 1, 2")) {
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[002]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT 1 AS a, 2")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[004]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT 1 AS a, 2, 2 as '2'")) {
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[006]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT 1 AS a, 2 as '2', 2")) {
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[008]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	/* Now do it with unbuffered queries */
	if (!$res = mysqli_real_query($link, "SELECT 1, 2")) {
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_use_result($link)) {
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[011]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_real_query($link, "SELECT 1 AS a, 2")) {
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_use_result($link)) {
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[014]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_real_query($link, "SELECT 1 AS a, 2, 2 as '2'")) {
		printf("[015] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_use_result($link)) {
		printf("[016] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[017]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_real_query($link, "SELECT 1 AS a, 2 as '2', 2")) {
		printf("[015] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_use_result($link)) {
		printf("[016] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[017]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);
	mysqli_close($link);

	print "done!";
?>