<?php
	require_once("connect.inc");

	$mysqli = new mysqli($host, $user, $passwd, $db);

	$read_stmt = $mysqli->prepare("SELECT 1");

	var_dump($read_stmt->bind_result($data));

	unset($mysqli);
	var_dump($read_stmt->bind_result($data));
	
?>
done!