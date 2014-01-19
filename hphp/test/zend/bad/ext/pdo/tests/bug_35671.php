<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test (field1 VARCHAR(32), field2 VARCHAR(32), field3 VARCHAR(32))');
$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

$insert = $db->prepare("insert into test (field1, field2, field3) values (:value1, :value2, :value3)");

$parm = array(
	":value1" => 15,
	":value2" => 20,
	":value3" => 25
);

$insert->execute($parm);
$insert = null;

var_dump($db->query("SELECT * from test")->fetchAll(PDO::FETCH_ASSOC));

?>