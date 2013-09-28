<?php

if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

@$db->exec("SET NAMES 'LATIN1'"); // needed for PostgreSQL
$db->exec("CREATE TABLE test (id INTEGER)");
$db->exec("INSERT INTO test (id) VALUES (1)");

$stmt = $db->prepare("SELECT 'Ã' as test FROM test WHERE id = :id");
$stmt->execute(array(":id" => 1));

$row = $stmt->fetch(PDO::FETCH_NUM);
var_dump( $row );

?>