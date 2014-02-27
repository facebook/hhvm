<?php
	require_once("connect.inc");
	$test_table_name = 'test_bug45019_table_1'; require_once("table.inc");

	// Regular (non-prepared) queries
	print "Using CAST('somestring' AS CHAR)...\n";
	if (!($res = $link->query("SELECT CAST('one' AS CHAR) AS column1 UNION SELECT CAST('three' AS CHAR) UNION SELECT CAST('two' AS CHAR)")))
		printf("[001] [%d] %s\n", $link->errno, $link->error);

	$data = array();
	while ($row = $res->fetch_assoc()) {
		$data[] = $row['column1'];
		var_dump($row['column1']);
	}
	$res->free();

	// Prepared Statements
	if (!($stmt = $link->prepare("SELECT CAST('one' AS CHAR) AS column1 UNION SELECT CAST('three' AS CHAR) UNION SELECT CAST('two' AS CHAR)")))
		printf("[002] [%d] %s\n", $link->errno, $link->error);

	$column1 = null;
	if (!$stmt->bind_result($column1) || !$stmt->execute())
		printf("[003] [%d] %s\n", $stmt->errno, $stmt->error);

	$index = 0;
	while ($stmt->fetch()) {
		/* NOTE: libmysql - http://bugs.mysql.com/bug.php?id=47483 */
		if ($data[$index] != $column1) {
			if ($IS_MYSQLND || $index != 1) {
				printf("[004] Row %d, expecting %s/%s got %s/%s\n",
					$index + 1, gettype($data[$index]), $data[$index], gettype($column1), $column1);
			} else {
				if ($column1 != "thre")
					printf("[005] Got '%s'. Please check if http://bugs.mysql.com/bug.php?id=47483 has been fixed and adapt tests bug45019.phpt/mysqli_ps_select_union.phpt", $column1);
			}
		}
		$index++;
	}
	$stmt->close();

	$link->close();

	print "done!";
?>