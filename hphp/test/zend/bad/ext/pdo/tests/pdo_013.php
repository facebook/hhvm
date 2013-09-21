<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), grp VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'B\', \'Group2\')'); 

$SELECT = 'SELECT val, grp FROM test';

$stmt = $db->prepare($SELECT);

$stmt->execute();
$stmt->setFetchMode(PDO::FETCH_NUM);
foreach ($stmt as $data)
{
	var_dump($data);
}

class Test
{
	function __construct($name = 'N/A')
	{
		echo __METHOD__ . "($name)\n";
	}
}

unset($stmt);

foreach ($db->query($SELECT, PDO::FETCH_CLASS, 'Test') as $data)
{
	var_dump($data);
}

unset($stmt);

$stmt = $db->query($SELECT, PDO::FETCH_CLASS, 'Test', array('WOW'));

foreach($stmt as $data)
{
	var_dump($data);
}
?>