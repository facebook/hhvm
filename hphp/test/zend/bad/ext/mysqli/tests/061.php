<?php
	require_once("connect.inc");

	function my_read($fp, &$buffer, $buflen, &$error) {
		$buffer = strrev(fread($fp, $buflen));
		return(strlen($buffer));
	}

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	/* create temporary file */
	$filename = dirname(__FILE__) . "061.csv";
	$fp = fopen($filename, "w");
	fwrite($fp, b"foo;bar");
	fclose($fp);

	if (!mysqli_query($link,"DROP TABLE IF EXISTS test_061_table_1"))
		printf("Cannot drop table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	if (!mysqli_query($link,"CREATE TABLE test_061_table_1 (c1 varchar(10), c2 varchar(10))"))
		printf("Cannot create table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, sprintf("LOAD DATA LOCAL INFILE '%s' INTO TABLE test_061_table_1 FIELDS TERMINATED BY ';'", mysqli_real_escape_string($link, $filename))))
		printf("Cannot load data: [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_set_local_infile_handler($link, "my_read");
	if (!mysqli_query($link, sprintf("LOAD DATA LOCAL INFILE '%s' INTO TABLE test_061_table_1 FIELDS TERMINATED BY ';'", mysqli_real_escape_string($link, $filename))))
		printf("Cannot load data using infile handler: [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if ($result = mysqli_query($link, "SELECT c1,c2 FROM test_061_table_1")) {
		while (($row = mysqli_fetch_row($result))) {
			printf("%s-%s\n", $row[0], $row[1]);
			printf("%s-%s\n", gettype($row[0]), gettype($row[1]));
		}
		mysqli_free_result($result);
	}

	mysqli_close($link);
	unlink($filename);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_061_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>