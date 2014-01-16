<?php
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

$db->exec('CREATE TABLE test (bar INT NOT NULL)');
$db->exec('INSERT INTO test VALUES(1)');

var_dump($db->query('SELECT * from test'));
foreach ($db->query('SELECT * from test') as $row) {
	print_r($row);
}

$stmt = $db->prepare('SELECT * from test');
print_r($stmt->getColumnMeta(0));
$stmt->execute();
$tmp = $stmt->getColumnMeta(0);

// libmysql and mysqlnd will show the pdo_type entry at a different position in the hash
if (!isset($tmp['pdo_type']) || (isset($tmp['pdo_type']) && $tmp['pdo_type'] != 2))
	printf("Expecting pdo_type = 2 got %s\n", $tmp['pdo_type']);
else
	unset($tmp['pdo_type']);

print_r($tmp);
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>