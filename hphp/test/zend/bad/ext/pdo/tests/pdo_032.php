<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10))');
$db->exec("INSERT INTO test VALUES(1, 'A')");
$db->exec("INSERT INTO test VALUES(2, 'B')");
$db->exec("INSERT INTO test VALUES(3, 'C')");

// Lower case columns
$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER);
$stmt = $db->prepare('SELECT * from test');
$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
$stmt->closeCursor();

// Upper case columns
$db->setAttribute(PDO::ATTR_CASE, PDO::CASE_UPPER);
$stmt = $db->prepare('SELECT * from test');
$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
$stmt->closeCursor();

?>