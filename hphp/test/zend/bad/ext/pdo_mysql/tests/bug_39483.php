<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, TRUE);
$stmt = $db->prepare('SELECT UPPER(\'\0:D\0\'),?');
$stmt->execute(array(1));
var_dump($stmt->fetchAll(PDO::FETCH_NUM));
