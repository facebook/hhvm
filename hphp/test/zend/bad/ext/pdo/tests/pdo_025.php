<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id INT NOT NULL PRIMARY KEY, val VARCHAR(10), val2 VARCHAR(16))');

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

class Test {
	public $id, $val, $val2;
}

$stmt = $db->prepare('SELECT * FROM test');
$stmt->setFetchMode(PDO::FETCH_INTO, new Test);
$stmt->execute();

foreach($stmt as $obj) {
	var_dump($obj);
}

echo "===FAIL===\n";

class Fail {
	protected $id;
	public $val, $val2;
}

$stmt->setFetchMode(PDO::FETCH_INTO, new Fail);
$stmt->execute();

foreach($stmt as $obj) {
	var_dump($obj);
}

?>