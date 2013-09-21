<?php

if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test (field1 VARCHAR(32), field2 VARCHAR(32), field3 VARCHAR(32), field4 INT)');

$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, true);
$s = $db->prepare("INSERT INTO test VALUES( ':id', 'name', 'section', 22)" );
$s->execute();

echo "Done\n";
?>