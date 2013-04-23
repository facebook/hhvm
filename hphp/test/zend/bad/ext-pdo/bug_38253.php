<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$pdo = PDOTest::factory();

$pdo->exec ("create table test (id integer primary key, n varchar(255))");
$pdo->exec ("INSERT INTO test (id, n) VALUES (1, 'hi')");

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_CLASS);
$stmt = $pdo->prepare ("SELECT * FROM test");
$stmt->execute();
var_dump($stmt->fetchAll());

$pdo = PDOTest::factory();

if ($pdo->getAttribute(PDO::ATTR_DRIVER_NAME) == 'oci') {
    $type = "clob";
} else{
    $type = "text";
}

$pdo->exec ("create table test2 (id integer primary key, n $type)");
$pdo->exec ("INSERT INTO test2 (id, n) VALUES (1,'hi')");

$pdo->setAttribute (PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_FUNC);
$stmt = $pdo->prepare ("SELECT * FROM test2");
$stmt->execute();
var_dump($stmt->fetchAll());

?>