<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();

$db->exec('DROP PROCEDURE IF EXISTS p');
$db->exec('CREATE PROCEDURE p() BEGIN SELECT 1 AS "one"; END');

$stmt = $db->query("CALL p()");
do {
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
} while ($stmt->nextRowset());
var_dump($stmt->errorInfo());

$stmt = $db->query('SELECT 2 AS "two"');
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
var_dump($stmt->errorInfo());
print "done!";
?>