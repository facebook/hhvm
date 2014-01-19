<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/'); 
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';

$db = PDOTest::factory();
$db->exec("CREATE TABLE test (a INT, b INT, c INT)");
$s = $db->prepare("INSERT INTO test (a,b,c) VALUES (:a,:b,:c)");

$s->execute(array('a' => 1, 'b' => 2, 'c' => 3));

@$s->execute(array('a' => 5, 'b' => 6, 'c' => 7, 'd' => 8));

$s->execute(array('a' => 9, 'b' => 10, 'c' => 11));

var_dump($db->query("SELECT * FROM test")->fetchAll(PDO::FETCH_ASSOC));
?>
===DONE===