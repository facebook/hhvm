<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec("CREATE TABLE test (x int)");
$db->exec("INSERT INTO test VALUES (1)");


// Bug entry [1]
$stmt = $db->query();
var_dump($stmt);


// Bug entry [2] -- 1 is PDO::FETCH_LAZY
$stmt = $db->query("SELECT * FROM test", PDO::FETCH_LAZY, 0, 0);
var_dump($stmt);


// Bug entry [3]
$stmt = $db->query("SELECT * FROM test", 'abc');
var_dump($stmt);


// Bug entry [4]
$stmt = $db->query("SELECT * FROM test", PDO::FETCH_CLASS, 0, 0, 0);
var_dump($stmt);


// Bug entry [5]
$stmt = $db->query("SELECT * FROM test", PDO::FETCH_INTO);
var_dump($stmt);


// Bug entry [6]
$stmt = $db->query("SELECT * FROM test", PDO::FETCH_COLUMN);
var_dump($stmt);


// Bug entry [7]
$stmt = $db->query("SELECT * FROM test", PDO::FETCH_CLASS);
var_dump($stmt);


?>