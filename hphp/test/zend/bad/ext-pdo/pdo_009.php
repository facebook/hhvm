<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE classtypes(id int NOT NULL PRIMARY KEY, name VARCHAR(10) NOT NULL UNIQUE)');
$db->exec('INSERT INTO classtypes VALUES(0, \'stdClass\')'); 
$db->exec('INSERT INTO classtypes VALUES(1, \'Test1\')'); 
$db->exec('INSERT INTO classtypes VALUES(2, \'Test2\')'); 
$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, classtype int, val VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, 0, \'A\')'); 
$db->exec('INSERT INTO test VALUES(2, 1, \'B\')'); 
$db->exec('INSERT INTO test VALUES(3, 2, \'C\')'); 
$db->exec('INSERT INTO test VALUES(4, 3, \'D\')'); 

$stmt = $db->prepare('SELECT classtypes.name, test.id AS id, test.val AS val FROM test LEFT JOIN classtypes ON test.classtype=classtypes.id');

class Test1
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

class Test2
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

class Test3
{
	public function __construct()
	{
		echo __METHOD__ . "()\n";
	}
}

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_NUM));

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_CLASS|PDO::FETCH_CLASSTYPE, 'Test3'));

?>