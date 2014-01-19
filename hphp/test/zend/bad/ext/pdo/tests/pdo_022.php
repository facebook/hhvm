<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id INT NOT NULL PRIMARY KEY, val VARCHAR(10), val2 VARCHAR(16))');
$db->exec('insert2', "INSERT INTO test VALUES(:first, :second, :third)"); 

$data = array(
    array('10', 'Abc', 'zxy'),
    array('20', 'Def', 'wvu'),
    array('30', 'Ghi', 'tsr'),
    array('40', 'Jkl', 'qpo'),
    array('50', 'Mno', 'nml'),
    array('60', 'Pqr', 'kji'),
);


// Insert using question mark placeholders
$stmt = $db->prepare("INSERT INTO test VALUES(?, ?, ?)");
foreach ($data as $row) {
    $stmt->execute($row);
}

// Retrieve column metadata for a result set returned by explicit SELECT
$select = $db->query('SELECT id, val, val2 FROM test');
$meta = $select->getColumnMeta(0);
var_dump($meta);
$meta = $select->getColumnMeta(1);
var_dump($meta);
$meta = $select->getColumnMeta(2);
var_dump($meta);

// Retrieve column metadata for a result set returned by a function
$select = $db->query('SELECT COUNT(*) FROM test');
$meta = $select->getColumnMeta(0);
var_dump($meta);

?>