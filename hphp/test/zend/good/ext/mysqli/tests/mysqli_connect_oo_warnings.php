<?php
	require_once("connect.inc");

	$myhost = 'invalidhost';
	$link   = NULL;

	print "1) bail\n";
	if (!is_object($mysqli = new mysqli($myhost)) || ('mysqli' !== get_class($mysqli)))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($mysqli), (is_object($mysqli)) ? var_export($mysqli, true) : $mysqli);

	print "2) be quiet\n";
	var_dump(mysqli_connect_error());
	var_dump(mysqli_connect_errno());

	print "3) bail\n";
	if (false !== ($link = mysqli_connect($myhost))) {
		printf("[003] Expecting boolean/false, got %s/%s\n", gettype($link), $link);
	}

	print "4) be quiet\n";
	if (false !== ($link = @mysqli_connect($myhost))) {
		printf("[004] Expecting boolean/false, got %s/%s\n", gettype($link), $link);
	}
	var_dump(mysqli_connect_error());
	var_dump(mysqli_connect_errno());

	print "done!";
?>