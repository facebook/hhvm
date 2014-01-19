<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'mysql')
	$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);

$db->exec("CREATE TABLE test (a varchar(100), b varchar(100), c varchar(100))");

for ($i = 0; $i < 5; $i++) {
	$db->exec("INSERT INTO test (a,b,c) VALUES('test".$i."','".$i."','".$i."')");
}

$stmt = $db->prepare("SELECT a FROM test WHERE b=:id-value");
$stmt->bindParam(':id-value', $id);
$id = '1';
$stmt->execute();
var_dump($stmt->fetch(PDO::FETCH_COLUMN));
?>