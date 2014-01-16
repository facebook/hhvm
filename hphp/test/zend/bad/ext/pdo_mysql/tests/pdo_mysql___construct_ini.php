<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

	$found = false;
	$values = ini_get_all();
	foreach ($values as $name => $dsn)
		if ('pdo.dsn.mysql' == $name) {
			printf("pdo.dsn.mysql=%s\n", $dsn);
			$found = true;
			break;
		}

	if (!$found) {
		$dsn = ini_get('pdo.dsn.mysql');
		$found = ($dsn !== false);
	}

	if (!$found)
		printf("pdo.dsn.mysql cannot be accessed through ini_get_all()/ini_get()\n");

	if (MySQLPDOTest::getDSN() == $dsn) {
		// we are lucky, we can run the test
		try {

			$user = PDO_MYSQL_TEST_USER;
			$pass	= PDO_MYSQL_TEST_PASS;
			$db = new PDO('mysql', $user, $pass);

		} catch (PDOException $e) {
			printf("[001] %s, [%s] %s\n",
				$e->getMessage(),
				(is_object($db)) ? $db->errorCode() : 'n/a',
				(is_object($db)) ? implode(' ', $db->errorInfo()) : 'n/a');
		}

	}

	print "done!";
?>