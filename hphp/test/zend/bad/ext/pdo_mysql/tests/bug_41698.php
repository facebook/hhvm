<?php
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

setlocale(LC_ALL, "de","de_DE","de_DE.ISO8859-1","de_DE.ISO_8859-1","de_DE.UTF-8");

$db->exec('CREATE TABLE test(floatval DECIMAL(8,6))');
$db->exec('INSERT INTO test VALUES(2.34)');
$value=4.56;
$stmt = $db->prepare('INSERT INTO test VALUES(?)');
$stmt->execute(array($value));
var_dump($db->query('SELECT * from test')->fetchAll(PDO::FETCH_ASSOC));
?>