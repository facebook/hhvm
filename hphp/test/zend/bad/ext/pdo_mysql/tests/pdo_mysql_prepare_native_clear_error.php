<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();

	try {

		$db->exec('DROP TABLE IF EXISTS test');
		$db->exec(sprintf('CREATE TABLE test(id INT, label CHAR(255)) ENGINE=%s', PDO_MYSQL_TEST_ENGINE));

		// We need to run the emulated version first. Native version will cause a fatal error
		$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 1);
		if (1 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
			printf("[002] Unable to turn on emulated prepared statements\n");

		// INSERT a single row
		$db->exec("INSERT INTO test(id, label) VALUES (1, 'row1')");

		$stmt = $db->prepare('SELECT unknown_column FROM test WHERE id > :placeholder ORDER BY id ASC');
		$stmt->execute(array(':placeholder' => 0));
		if ('00000' !== $stmt->errorCode())
			printf("[003] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));

		$stmt = $db->prepare('SELECT id, label FROM test WHERE id > :placeholder ORDER BY id ASC');
		$stmt->execute(array(':placeholder' => 0));
		if ('00000' !== $stmt->errorCode())
			printf("[004] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

		// Native PS
		$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 0);
		if (0 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
			printf("[005] Unable to turn off emulated prepared statements\n");

		$stmt = $db->prepare('SELECT unknown_column FROM test WHERE id > :placeholder ORDER BY id ASC');
		$stmt->execute(array(':placeholder' => 0));
		if ('00000' !== $stmt->errorCode())
			printf("[006] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));

		$stmt = $db->prepare('SELECT id, label FROM test WHERE id > :placeholder ORDER BY id ASC');
		$stmt->execute(array(':placeholder' => 0));
		if ('00000' !== $stmt->errorCode())
			printf("[007] Execute has failed, %s %s\n",
				var_export($stmt->errorCode(), true),
				var_export($stmt->errorInfo(), true));
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));


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