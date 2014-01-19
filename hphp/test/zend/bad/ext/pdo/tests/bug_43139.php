<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->setAttribute(PDO::ATTR_DEFAULT_FETCH_MODE, PDO::FETCH_ASSOC);
$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_WARNING);

$from = '';
if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'oci') {
	$from = 'from dual';
} else if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'firebird') {
	$from = 'FROM RDB$DATABASE';
}

var_dump($db->query("select 0 as abc, 1 as xyz, 2 as def $from")->fetchAll(PDO::FETCH_GROUP));
?>