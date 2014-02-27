<?php
	include ("connect.inc");

	class my_stmt extends mysqli_stmt
	{
		public function __construct($link, $query) {
			parent::__construct($link, $query);
		}
	}

	class my_result extends mysqli_result
	{
		public function __construct($link, $query) {
			parent::__construct($link, $query);
		}
	}

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);
	mysqli_query($link, "SET sql_mode=''");

	$stmt = new my_stmt($link, "SELECT 'foo' FROM test_bug34785_table_1");

	$stmt->execute();
	$stmt->bind_result($var);
	$stmt->fetch();

	$stmt->close();
	var_dump($var);

	mysqli_real_query($link, "SELECT 'bar' FROM test_bug34785_table_1");
	$result = new my_result($link, MYSQLI_STORE_RESULT);
	$row = $result->fetch_row();
	$result->close();

	var_dump($row[0]);

	mysqli_close($link);
?>