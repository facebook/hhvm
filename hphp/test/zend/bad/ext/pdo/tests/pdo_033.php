<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$unquoted = ' !"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~';

$quoted = $db->quote($unquoted);

$len = strlen($unquoted);

@$db->exec("DROP TABLE test");

$db->query("CREATE TABLE test (t char($len))");
$db->query("INSERT INTO test (t) VALUES($quoted)");

$stmt = $db->prepare('SELECT * from test');
$stmt->execute();

print_r($stmt->fetchAll(PDO::FETCH_ASSOC));

$db->exec("DROP TABLE test");

?>