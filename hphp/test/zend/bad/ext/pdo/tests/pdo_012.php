<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), grp VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'B\', \'Group2\')'); 

$SELECT = 'SELECT val, grp FROM test';

$stmt = $db->query($SELECT, PDO::FETCH_NUM);
var_dump($stmt->fetchAll());

class Test
{
	function __construct($name = 'N/A')
	{
		echo __METHOD__ . "($name)\n";
	}
}

unset($stmt);

$stmt = $db->query($SELECT, PDO::FETCH_CLASS, 'Test');
var_dump($stmt->fetchAll());

unset($stmt);

$stmt = $db->query($SELECT, PDO::FETCH_NUM);
$stmt->setFetchMode(PDO::FETCH_CLASS, 'Test', array('Changed'));
var_dump($stmt->fetchAll());

?>