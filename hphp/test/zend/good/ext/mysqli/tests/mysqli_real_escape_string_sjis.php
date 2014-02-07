<?php

	require_once("connect.inc");
	$test_table_name = 'test_mysqli_real_escape_string_sjis_table_1'; require_once('table.inc');

	var_dump(mysqli_set_charset($link, "sjis"));

	if ('?p??\\\\?p??' !== ($tmp = mysqli_real_escape_string($link, '?p??\\?p??')))
		printf("[004] Expecting \\\\, got %s\n", $tmp);

	if ('?p??\"?p??' !== ($tmp = mysqli_real_escape_string($link, '?p??"?p??')))
		printf("[005] Expecting \", got %s\n", $tmp);

	if ("?p??\'?p??" !== ($tmp = mysqli_real_escape_string($link, "?p??'?p??")))
		printf("[006] Expecting ', got %s\n", $tmp);

	if ("?p??\\n?p??" !== ($tmp = mysqli_real_escape_string($link, "?p??\n?p??")))
		printf("[007] Expecting \\n, got %s\n", $tmp);

	if ("?p??\\r?p??" !== ($tmp = mysqli_real_escape_string($link, "?p??\r?p??")))
		printf("[008] Expecting \\r, got %s\n", $tmp);

	if ("?p??\\0?p??" !== ($tmp = mysqli_real_escape_string($link, "?p??" . chr(0) . "?p??")))
		printf("[009] Expecting %s, got %s\n", "?p??\\0?p??", $tmp);

	var_dump(mysqli_query($link, "INSERT INTO test_mysqli_real_escape_string_sjis_table_1(id, label) VALUES (100, '?p')"));

	mysqli_close($link);
	print "done!";
?>