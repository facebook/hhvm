<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

	function mysql_stmt_multiquery_wrong_usage($db) {

		$stmt = $db->query('SELECT label FROM test ORDER BY id ASC LIMIT 1; SELECT label FROM test ORDER BY id ASC LIMIT 1');
		var_dump($stmt->errorInfo());
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
		var_dump($stmt->errorInfo());

	}

	function mysql_stmt_multiquery_proper_usage($db) {

		$stmt = $db->query('SELECT label FROM test ORDER BY id ASC LIMIT 1; SELECT label FROM test ORDER BY id ASC LIMIT 1');
		do {
			var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
		} while ($stmt->nextRowset());

	}

	try {

		printf("Emulated Prepared Statements...\n");
		$db = MySQLPDOTest::factory();
		MySQLPDOTest::createTestTable($db);
		$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
		mysql_stmt_multiquery_wrong_usage($db);
		mysql_stmt_multiquery_proper_usage($db);

		printf("Native Prepared Statements...\n");
		$db = MySQLPDOTest::factory();
		MySQLPDOTest::createTestTable($db);
		$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
		mysql_stmt_multiquery_wrong_usage($db);
		mysql_stmt_multiquery_proper_usage($db);

	} catch (PDOException $e) {
		printf("[001] %s [%s] %s\n",
			$e->getMessage(), $db->errorCode(), implode(' ', $db->errorInfo()));
	}

	print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>