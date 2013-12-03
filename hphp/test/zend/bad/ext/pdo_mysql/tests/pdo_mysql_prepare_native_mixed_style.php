<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	MySQLPDOTest::createTestTable($db);

	$db->setAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY, 0);
	if (0 != $db->getAttribute(PDO::MYSQL_ATTR_DIRECT_QUERY))
		printf("[002] Unable to turn off emulated prepared statements\n");

	$stmt = $db->query('DELETE FROM test');
	$stmt = $db->prepare('INSERT INTO test(id, label) VALUES (1, ?), (2, ?)');
	$stmt->execute(array('a', 'b'));
	$stmt = $db->prepare("SELECT id, label FROM test WHERE id = :placeholder AND label = (SELECT label AS 'SELECT' FROM test WHERE id = ?)");
	$stmt->execute(array(1, 1));
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>