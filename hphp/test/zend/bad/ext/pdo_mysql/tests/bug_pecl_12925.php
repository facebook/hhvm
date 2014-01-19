<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();

function bug_pecl_1295($db) {

	$db->exec('DROP TABLE IF EXISTS test');
	$db->exec('CREATE TABLE test(id CHAR(1))');
	$db->exec("INSERT INTO test(id) VALUES ('a')");
	$stmt = $db->prepare("UPDATE test SET id = 'b'");
	$stmt->execute();
	$stmt = $db->prepare("UPDATE test SET id = 'c'");
	$stmt->execute();
	$stmt = $db->prepare('SELECT id FROM test');
	$stmt->execute();
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
	$stmt->closeCursor();

}

printf("Emulated...\n");
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
bug_pecl_1295($db);

printf("Native...\n");
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
bug_pecl_1295($db);

$db->exec('DROP TABLE IF EXISTS test');
print "done!";
?>