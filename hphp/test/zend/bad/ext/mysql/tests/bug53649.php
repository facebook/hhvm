<?php
	require_once("connect.inc");

	if (!$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysqlerrno(), mysqli_error());
	}

	if (!mysql_query("DROP TABLE IF EXISTS test", $link)) {
		printf("[002] [%d] %s\n", mysql_errno($link), mysql_error($link));
	}

	if (!mysql_query("CREATE TABLE test (dump1 INT UNSIGNED NOT NULL PRIMARY KEY) ENGINE=" . $engine, $link)) {
		printf("[003] [%d] %s\n", mysql_errno($link), mysql_error($link));
	}

	if (false === file_put_contents('bug53649.data', "1\n2\n3\n"))
		printf("[004] Failed to create data file\n");

	if (!mysql_query("SELECT 1 FROM DUAL", $link))
	  printf("[005] [%d] %s\n", mysql_errno($link), mysql_error($link));

	if (!mysql_query("LOAD DATA LOCAL INFILE 'bug53649.data' INTO TABLE test", $link)) {
		printf("[006] [%d] %s\n", mysql_errno($link), mysql_error($link));
		echo "bug";
	} else {
		echo "done";
	}
	mysql_close($link);
?>
<?php
require_once('connect.inc');

if (!$link = my_mysql_connect($host, $user, $passwd, $db, $port, $socket)) {
	printf("[clean] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
		$host, $user, $db, $port, $socket);
}

if (!mysql_query($link, 'DROP TABLE IF EXISTS test', $link)) {
	printf("[clean] Failed to drop old test table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
}

mysql_close($link);

unlink('bug53649.data');
?>
