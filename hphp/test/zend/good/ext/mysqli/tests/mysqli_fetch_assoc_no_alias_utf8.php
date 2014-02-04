<?php
	$test_table_name = 'test_mysqli_fetch_assoc_no_alias_utf8_table_1'; require('table.inc');

	/* some cyrillic (utf8) comes here */
	if (!$res = mysqli_query($link, "SET NAMES UTF8")) {
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!$res = mysqli_query($link, "SELECT 1 AS 'Андрей Христов', 2 AS 'Улф Вендел', 3 AS 'Георг Рихтер'")) {
		printf("[002] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[003]\n";
	var_dump(mysqli_fetch_assoc($res));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "CREATE TABLE автори_на_mysqlnd (id integer not null auto_increment primary key, име varchar(20) character set ucs2, фамилия varchar(20) character set utf8)")) {
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_query($link, "INSERT INTO автори_на_mysqlnd (име, фамилия) VALUES ('Андрей', 'Христов'), ('Георг', 'Рихтер'), ('Улф','Вендел')")) {
		printf("[005] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_query($link, "INSERT INTO автори_на_mysqlnd (име, фамилия) VALUES ('Andrey', 'Hristov'), ('Georg', 'Richter'), ('Ulf','Wendel')")) {
		printf("[006] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	if (!$res = mysqli_query($link, "INSERT INTO автори_на_mysqlnd (име, фамилия) VALUES ('安德烈', 'Hristov'), ('格奥尔', 'Richter'), ('乌尔夫','Wendel')")) {
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!$res = mysqli_query($link, "SELECT id, име, фамилия FROM автори_на_mysqlnd ORDER BY фамилия, име")) {
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[009]\n";
	while ($row = mysqli_fetch_assoc($res)) {
		var_dump($row);
	}
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "DROP TABLE автори_на_mysqlnd")) {
		printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_close($link);
	print "done!";
?>