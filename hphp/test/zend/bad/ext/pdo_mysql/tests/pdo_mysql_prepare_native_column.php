<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	MySQLPDOTest::createTestTable($db);

	$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 0);
	if (0 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
		printf("[002] Unable to turn off emulated prepared statements\n");

	$stmt = $db->prepare("SELECT :param FROM test ORDER BY id ASC LIMIT 1");
	$stmt->execute(array(':param' => 'id'));
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	$db->prepare('SELECT :placeholder FROM test WHERE :placeholder > :placeholder');
	$stmt->execute(array(':placeholder' => 'test'));

	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec('DROP TABLE IF EXISTS test');
?>