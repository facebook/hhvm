<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();
$db->exec("DROP TABLE IF EXISTS test");

// And now allow the evil to do his work
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
$sql = "CREATE TABLE IF NOT EXISTS test(id INT); INSERT INTO test(id) VALUES (1); SELECT * FROM test; INSERT INTO test(id) VALUES (2); SELECT * FROM test;";
// NOTE: This will fail, it is OK to fail - you must not mix DML/DDL and SELECT
// The PDO API does not support multiple queries properly!
// Read http://blog.ulf-wendel.de/?p=192
// Compare MySQL C-API documentation
$stmt = $db->query($sql);
do {
	var_dump($stmt->fetchAll());
} while ($stmt->nextRowset());

print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec("DROP TABLE IF EXISTS test");
?>