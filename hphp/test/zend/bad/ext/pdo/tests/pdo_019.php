<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(idx int NOT NULL PRIMARY KEY, txt VARCHAR(20))');
$db->exec('INSERT INTO test VALUES(0, \'String0\')'); 
$db->exec('INSERT INTO test VALUES(1, \'String1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'String2\')'); 
$db->exec('INSERT INTO test VALUES(3, \'String3\')'); 


var_dump($db->query('SELECT COUNT(*) FROM test')->fetchColumn());

$stmt = $db->prepare('SELECT idx, txt FROM test ORDER by idx');

$stmt->execute();
$cont = $stmt->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE);
var_dump($cont);

echo "===WHILE===\n";

$stmt->bindColumn('idx', $idx);
$stmt->bindColumn('txt', $txt);
$stmt->execute();

while($stmt->fetch(PDO::FETCH_BOUND)) {
	var_dump(array($idx=>$txt));
}

?>