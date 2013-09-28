<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();

function bug_42499($db) {

	$db->exec('DROP TABLE IF EXISTS test');
	$db->exec("CREATE TABLE test(id CHAR(1)); INSERT INTO test(id) VALUES ('a')");

	$stmt = $db->query('SELECT id AS _id FROM test');
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	// You must not use exec() to run statements that create a result set!
	$db->exec('SELECT id FROM test');
	// This will bail at you because you have not fetched the SELECT results: this is not a bug!
	$db->exec("INSERT INTO test(id) VALUES ('b')");

}

print "Emulated Prepared Statements...\n";
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
$db->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY, 1);
bug_42499($db);

print "Native Prepared Statements...\n";
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
$db->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY, 1);
bug_42499($db);

$db = MySQLPDOTest::factory();
$db->exec('DROP TABLE IF EXISTS test');

print "done!";
?>