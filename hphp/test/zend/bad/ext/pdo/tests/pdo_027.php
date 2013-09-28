<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('create table test (id int, name varchar(10))');
$db->exec("INSERT INTO test (id,name) VALUES(1,'test1')");
$db->exec("INSERT INTO test (id,name) VALUES(2,'test2')");

foreach ($db->query("SELECT * FROM test", PDO::FETCH_LAZY) as $v) {
	echo "lazy: " . $v->id.$v->name."\n";
}
echo "End\n";
?>