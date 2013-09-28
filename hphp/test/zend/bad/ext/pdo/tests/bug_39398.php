<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/'); 
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';

$db = PDOTest::factory();
$db->exec("CREATE TABLE test (test INT)");

$boolean = 1;
$stmt = $db->prepare('INSERT INTO test VALUES (:boolean)');
$stmt->bindValue(':boolean', isset($boolean), PDO::PARAM_INT);
$stmt->execute();

var_dump($db->query("SELECT * FROM test")->fetchAll(PDO::FETCH_ASSOC));
?>
===DONE===