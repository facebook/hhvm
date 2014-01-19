<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

print_r($db->query("CREATE TABLE test (id int auto_increment primary key, num int)"));

print_r($db->query("INSERT INTO test (id, num) VALUES (23, 42)"));

print_r($db->query("INSERT INTO test (num) VALUES (451)"));

print_r($db->lastInsertId());