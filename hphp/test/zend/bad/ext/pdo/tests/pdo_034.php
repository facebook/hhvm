<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec("CREATE TABLE test (a varchar(100), b varchar(100), c varchar(100))");

for ($i = 0; $i < 5; $i++) {
	$db->exec("INSERT INTO test (a,b,c) VALUES('test".$i."','".$i."','".$i."')");
}

var_dump($db->query("SELECT a,b FROM test")->fetch(PDO::FETCH_KEY_PAIR));
var_dump($db->query("SELECT a,b FROM test")->fetchAll(PDO::FETCH_KEY_PAIR));
var_dump($db->query("SELECT * FROM test")->fetch(PDO::FETCH_KEY_PAIR));
var_dump($db->query("SELECT a,a FROM test")->fetchAll(PDO::FETCH_KEY_PAIR));

?>