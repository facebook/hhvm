<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_SILENT);
$x = $db->query("UPDATE non_existent_pdo_test_table set foo = 'bar'");

var_dump($x);
$code = $db->errorCode();
if ($code !== '00000' && strlen($code)) {
	echo "OK: $code\n";
} else {
	echo "ERR: $code\n";
	print_r($db->errorInfo());
}

?>