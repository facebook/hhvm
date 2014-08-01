<?php
	include ("connect.inc");

	$link1 = my_mysqli_connect($host, $user, $passwd, null, $port, $socket);
	mysqli_select_db($link1, $db);

	$link1->query("SELECT 'test'", MYSQLI_ASYNC);
	$all_links = array($link1);
	$links = $errors = $reject = $all_links;
	mysqli_poll($links, $errors, $reject, 1);

	echo "links: ",     sizeof($links), "\n";
	echo "errors: ",    sizeof($errors), "\n";
	echo "reject: ",    sizeof($reject), "\n";
	echo "all_links: ", sizeof($all_links), "\n";

	$link1->close();
?>