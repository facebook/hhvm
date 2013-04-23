<?php

if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

@$db->exec("DROP TABLE test");
$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

$db->exec("CREATE TABLE test (id INTEGER NOT NULL PRIMARY KEY, usr VARCHAR( 256 ) NOT NULL)");
$db->exec("INSERT INTO test (id, usr) VALUES (1, 'user')");

$stmt = $db->prepare("SELECT * FROM test WHERE id = ?");
$stmt->bindValue(1, 1, PDO::PARAM_INT );
$stmt->execute();
$row = $stmt->fetch();
var_dump( $row );

$stmt->execute();
$stmt->closeCursor();
$row = $stmt->fetch(); // this line will crash CLI
var_dump( $row );

@$db->exec("DROP TABLE test");
echo "Done\n";
?>