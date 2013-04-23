<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), val2 VARCHAR(20))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'A2\')'); 
$db->exec('INSERT INTO test VALUES(2, \'A\', \'B2\')'); 

$select1 = $db->prepare('SELECT id, val, val2 FROM test');
$select2 = $db->prepare('SELECT val, val2 FROM test');

$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN, 2));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_GROUP));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE, 0));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE, 1));
$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE, 2));

$select2->execute();
var_dump($select2->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_GROUP));

?>