<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');
$query = $db->prepare('SELECT 1 AS num');
$query->execute();
if(!is_array($query->getColumnMeta(0))) die('FAIL!');
$query->nextRowset();
$query->execute();
if(!is_array($query->getColumnMeta(0))) die('FAIL!');
echo 'done!';
?>