<?php
	include ("connect.inc");

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[001] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test_bug49442_table_1')) {
		printf("[002] Failed to drop old test_bug49442_table_1 table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!mysqli_query($link, 'CREATE TABLE test_bug49442_table_1(id INT, label CHAR(1), PRIMARY KEY(id)) ENGINE=' . $engine)) {
		printf("[003] Failed to create test_bug49442_table_1 table: [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	include("local_infile_tools.inc");
	$file = create_standard_csv(4);

	if (!@mysqli_query($link, sprintf("LOAD DATA LOCAL INFILE '%s'
			INTO TABLE test_bug49442_table_1
			FIELDS TERMINATED BY ';' OPTIONALLY ENCLOSED BY '\''
			LINES TERMINATED BY '\n'",
			mysqli_real_escape_string($link, $file)))) {
			printf("[005] [%d] %s\n",  mysqli_errno($link), mysqli_error($link));
	}

	if (!$res = mysqli_query($link, "SELECT * FROM test_bug49442_table_1 ORDER BY id"))
		printf("[006] [%d] %s\n",  mysqli_errno($link), mysqli_error($link));

	$rows = array();
	while ($row = mysqli_fetch_assoc($res)) {
		var_dump($row);
		$rows[] = $row;
	}

	mysqli_free_result($res);

	mysqli_query($link, "DELETE FROM test_bug49442_table_1");
	mysqli_close($link);

	if ($IS_MYSQLND) {
		/*
			mysqlnd makes a connection created through mysql_init()/mysqli_real_connect() always a 'persistent' one.
			At this point 'persistent' is not to be confused with what a user calls a 'persistent' - in this case
			'persistent' means that mysqlnd uses malloc() instead of emalloc(). nothing else. ext/mysqli will
			not consider it as a 'persistent' connection in a user sense, ext/mysqli will not appy max_persistent etc.
			Its only about malloc() vs. emalloc().

			However, the bug is about malloc() and efree(). You can make make mysqlnd use malloc() by either using
			pconnect or mysql_init() - so we should test_bug49442_table_1 pconnect as well..
		*/
		$host = 'p:' . $host;
		if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket)) {
			printf("[007] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
		}

		/* bug happened during query processing */
		if (!@mysqli_query($link, sprintf("LOAD DATA LOCAL INFILE '%s'
			INTO TABLE test_bug49442_table_1
			FIELDS TERMINATED BY ';' OPTIONALLY ENCLOSED BY '\''
			LINES TERMINATED BY '\n'",
			mysqli_real_escape_string($link, $file)))) {
			printf("[008] [%d] %s\n",  mysqli_errno($link), mysqli_error($link));
		}

		/* we survived? that's good enough... */

		if (!$res = mysqli_query($link, "SELECT * FROM test_bug49442_table_1 ORDER BY id"))
			printf("[009] [%d] %s\n",  mysqli_errno($link), mysqli_error($link));

		$i = 0;
		while ($row = mysqli_fetch_assoc($res)) {
			if (($row['id'] != $rows[$i]['id']) || ($row['label'] != $rows[$i]['label'])) {
				printf("[010] Wrong values, check manually!\n");
			}
			$i++;
		}
		mysqli_close($link);
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_bug49442_table_1'; require_once("clean_table.inc");
?>