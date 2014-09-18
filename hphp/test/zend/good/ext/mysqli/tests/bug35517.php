<?php
	require_once("connect.inc");

	$mysql = new my_mysqli($host, $user, $passwd, $db, $port, $socket);

	$mysql->query("CREATE TABLE test_bug35517_table_1 (id INT UNSIGNED NOT NULL)");
	$mysql->query("INSERT INTO test_bug35517_table_1 (id) VALUES (3000000897),(3800001532),(3900002281),(3100059612)");
	$stmt = $mysql->prepare("SELECT id FROM test_bug35517_table_1");
	$stmt->execute();
	$stmt->bind_result($id);
	while ($stmt->fetch()) {
		if (PHP_INT_SIZE == 8) {
			if ((gettype($id) !== 'int') && (gettype($id) != 'integer'))
				printf("[001] Expecting integer on 64bit got %s/%s\n", gettype($id), var_export($id, true));
		} else {
			if (gettype($id) !== 'string') {
				printf("[002] Expecting string on 32bit got %s/%s\n", gettype($id), var_export($id, true));
			}
			if ((version_compare(PHP_VERSION, '5.9.9', '>') == 1) && !is_unicode($id)) {
				printf("[003] Expecting unicode string\n");
			}
		}
		print $id;
		print "\n";
	}
	$stmt->close();

	$mysql->query("DROP TABLE test_bug35517_table_1");
	$mysql->close();
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_bug35517_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_close($link);
?>