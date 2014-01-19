<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

	$dsn = MySQLPDOTest::getDSN();
	$user = PDO_MYSQL_TEST_USER;
	$pass = PDO_MYSQL_TEST_PASS;

	$table = sprintf("test_%s", md5(mt_rand(0, PHP_INT_MAX)));
	$db = new PDO($dsn, $user, $pass);
	$db->exec(sprintf('DROP TABLE IF EXISTS %s', $table));

	$create = sprintf('CREATE TABLE %s(id INT)', $table);
	var_dump($create);
	$db = new PDO($dsn, $user, $pass, array(PDO::MYSQL_ATTR_INIT_COMMAND => $create));

	$info = $db->errorInfo();
	var_dump($info[0]);

	$db->exec(sprintf('INSERT INTO %s(id) VALUES (1)', $table));
	$stmt = $db->query(sprintf('SELECT id FROM %s', $table));
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	$db->exec(sprintf('DROP TABLE IF EXISTS %s', $table));
	print "done!";
?>