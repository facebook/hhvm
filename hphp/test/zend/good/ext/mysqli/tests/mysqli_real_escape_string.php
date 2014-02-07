<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_real_escape_string_table_1'; require('table.inc');
	if ('\\\\' !== ($tmp = mysqli_real_escape_string($link, '\\')))
		printf("[004] Expecting \\\\, got %s\n", $tmp);

	if ('\"' !== ($tmp = mysqli_real_escape_string($link, '"')))
		printf("[005] Expecting \", got %s\n", $tmp);

	if ("\'" !== ($tmp = mysqli_real_escape_string($link, "'")))
		printf("[006] Expecting ', got %s\n", $tmp);

	if ("\\n" !== ($tmp = mysqli_real_escape_string($link, "\n")))
		printf("[007] Expecting \\n, got %s\n", $tmp);

	if ("\\r" !== ($tmp = mysqli_real_escape_string($link, "\r")))
		printf("[008] Expecting \\r, got %s\n", $tmp);

	if ("foo\\0bar" !== ($tmp = mysqli_real_escape_string($link, "foo" . chr(0) . "bar")))
		printf("[009] Expecting %s, got %s\n", "foo\\0bar", $tmp);

	mysqli_close($link);

	if (NULL !== ($tmp = mysqli_real_escape_string($link, 'foo')))
		printf("[010] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	/* Make sure that the function alias exists */
	print "done!";
?>