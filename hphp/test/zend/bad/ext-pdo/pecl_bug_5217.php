<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();
try {
	$ser = serialize($db);
	debug_zval_dump($ser);
	$db = unserialize($ser);
	$db->exec('CREATE TABLE test (id int NOT NULL PRIMARY KEY, val VARCHAR(10))');
} catch (Exception $e) {
	echo "Safely caught " . $e->getMessage() . "\n";
}

echo "PHP Didn't crash!\n";
?>