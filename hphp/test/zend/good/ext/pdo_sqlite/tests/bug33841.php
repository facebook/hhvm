<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

$db->exec('CREATE TABLE test (text)');

$stmt = $db->prepare("INSERT INTO test VALUES ( :text )");
$stmt->bindParam(':text', $name);
$name = 'test1';
var_dump($stmt->execute(), $stmt->rowCount());

$stmt = $db->prepare("UPDATE test SET text = :text ");
$stmt->bindParam(':text', $name);
$name = 'test2';
var_dump($stmt->execute(), $stmt->rowCount());
