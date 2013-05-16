<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'mysql') {
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$suf = ' ENGINE=' . MySQLPDOTest::detect_transactional_mysql_engine($db);
} else {
	$suf = '';
}

$db->exec('CREATE TABLE test(id INT NOT NULL PRIMARY KEY, val VARCHAR(10))'.$suf);
$db->exec("INSERT INTO test VALUES(1, 'A')");
$db->exec("INSERT INTO test VALUES(2, 'B')");
$db->exec("INSERT INTO test VALUES(3, 'C')");
$delete = $db->prepare('DELETE FROM test');

function countRows($action) {
    global $db;
	$select = $db->prepare('SELECT COUNT(*) FROM test');
	$select->execute();
    $res = $select->fetchColumn();
    return "Counted $res rows after $action.\n";
}

echo countRows('insert');

$db->beginTransaction();
$delete->execute();
echo countRows('delete');
$db->rollBack();

echo countRows('rollback');

$db->beginTransaction();
$delete->execute();
echo countRows('delete');
$db->commit();

echo countRows('commit');

?>