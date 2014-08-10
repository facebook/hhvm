<?php

	require_once("connect.inc");
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
{
		printf("[001] Cannot connect to the server using host=%s, user=%s,
passwd=***, dbname=%s, port=%s, socket=%s - [%d] %s\n", $host, $user, $db,
$port, $socket, mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test_mysqli_real_escape_string_gb2312_table_1')) {
		printf("Failed to drop old test_mysqli_real_escape_string_gb2312_table_1 table: [%d] %s\n", mysqli_errno($link),
mysqli_error($link));
	}

	if (!mysqli_query($link, 'CREATE TABLE test_mysqli_real_escape_string_gb2312_table_1(id INT, label CHAR(1), PRIMARY
KEY(id)) ENGINE=' . $engine . " DEFAULT CHARSET=gb2312")) {
		printf("Failed to create test_mysqli_real_escape_string_gb2312_table_1 table: [%d] %s\n", mysqli_errno($link),
mysqli_error($link));
	}

	var_dump(mysqli_set_charset($link, "gb2312"));

	if ('����\\\\����' !== ($tmp = mysqli_real_escape_string($link, '����\\����')))
		printf("[004] Expecting \\\\, got %s\n", $tmp);

	if ('����\"����' !== ($tmp = mysqli_real_escape_string($link, '����"����')))
		printf("[005] Expecting \", got %s\n", $tmp);

	if ("����\'����" !== ($tmp = mysqli_real_escape_string($link, "����'����")))
		printf("[006] Expecting ', got %s\n", $tmp);

	if ("����\\n����" !== ($tmp = mysqli_real_escape_string($link, "����\n����")))
		printf("[007] Expecting \\n, got %s\n", $tmp);

	if ("����\\r����" !== ($tmp = mysqli_real_escape_string($link, "����\r����")))
		printf("[008] Expecting \\r, got %s\n", $tmp);

	if ("����\\0����" !== ($tmp = mysqli_real_escape_string($link, "����" . chr(0) . "����")))
		printf("[009] Expecting %s, got %s\n", "����\\0����", $tmp);

	var_dump(mysqli_query($link, "INSERT INTO test_mysqli_real_escape_string_gb2312_table_1(id, label) VALUES (100, '��')"));

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_real_escape_string_gb2312_table_1'; require_once("clean_table.inc");
?>