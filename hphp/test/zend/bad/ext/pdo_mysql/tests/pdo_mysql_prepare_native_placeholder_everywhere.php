<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	try {
		$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 1);
		if (1 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
			printf("[002] Unable to switch on emulated prepared statements, test will fail\n");

		$db->exec('DROP TABLE IF EXISTS test');
		$db->exec(sprintf('CREATE TABLE test(id INT, label CHAR(255)) ENGINE=%s', PDO_MYSQL_TEST_ENGINE));
		$db->exec("INSERT INTO test(id, label) VALUES (1, 'row1')");

		$stmt = $db->prepare('SELECT ?, id, label FROM test WHERE ? = ? ORDER BY id ASC');
		$stmt->execute(array('id', 'label', 'label'));
		if ('00000' !== $stmt->errorCode())
			printf("[003] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

		// now the same with native PS
		printf("now the same with native PS\n");
		$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 0);
		if (0 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
			printf("[004] Unable to switch off emulated prepared statements, test will fail\n");

		$stmt = $db->prepare('SELECT ?, id, label FROM test WHERE ? = ? ORDER BY id ASC');
		$stmt->execute(array('id', 'label', 'label'));
		if ('00000' !== $stmt->errorCode())
			printf("[005] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));

		$tmp = $stmt->fetchAll(PDO::FETCH_ASSOC);
		if (!MySQLPDOTest::isPDOMySQLnd()) {
			if (isset($tmp[0]['id'])) {
				// libmysql should return a string here whereas mysqlnd returns a native int
				if (gettype($tmp[0]['id']) == 'string')
					// convert to int for the test output...
					settype($tmp[0]['id'], 'integer');
			}
		}
		var_dump($tmp);

	} catch (PDOException $e) {
		printf("[001] %s [%s] %s\n",
			$e->getMessage(), $db->errorCode(), implode(' ', $db->errorInfo()));
	}

	print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec('DROP TABLE IF EXISTS test');
?>