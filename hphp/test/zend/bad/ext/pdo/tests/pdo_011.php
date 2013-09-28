<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), grp VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'B\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(3, \'C\', \'Group2\')'); 
$db->exec('INSERT INTO test VALUES(4, \'D\', \'Group2\')'); 

class DerivedStatement extends PDOStatement
{
	private function __construct($name, $db)
	{
		$this->name = $name;
		echo __METHOD__ . "($name)\n";
	}

	function reTrieve($id, $val) {
		echo __METHOD__ . "($id,$val)\n";
		return array($id=>$val);
	}
}

$select1 = $db->prepare('SELECT grp, id FROM test');
$select2 = $db->prepare('SELECT id, val FROM test');
$derived = $db->prepare('SELECT id, val FROM test', array(PDO::ATTR_STATEMENT_CLASS=>array('DerivedStatement', array('Overloaded', $db))));

class Test1
{
	public function __construct($id, $val)
	{
		echo __METHOD__ . "($id,$val)\n";
		$this->id = $id;
		$this->val = $val;
	}

	static public function factory($id, $val)
	{
		echo __METHOD__ . "($id,$val)\n";
		return new self($id, $val);
	}
}

function test($id,$val='N/A')
{
	echo __METHOD__ . "($id,$val)\n";
	return array($id=>$val);
}

$f = new Test1(0,0);

$select1->execute();
var_dump($select1->fetchAll(PDO::FETCH_FUNC|PDO::FETCH_GROUP, 'test'));

$select2->execute();
var_dump($select2->fetchAll(PDO::FETCH_FUNC, 'test'));

$select2->execute();
var_dump($select2->fetchAll(PDO::FETCH_FUNC, array('Test1','factory')));

$select2->execute();
var_dump($select2->fetchAll(PDO::FETCH_FUNC, array($f, 'factory')));

var_dump(get_class($derived));
$derived->execute();
var_dump($derived->fetchAll(PDO::FETCH_FUNC, array($derived, 'retrieve')));
$derived->execute();
var_dump($derived->fetchAll(PDO::FETCH_FUNC, array($derived, 'reTrieve')));
$derived->execute();
var_dump($derived->fetchAll(PDO::FETCH_FUNC, array($derived, 'RETRIEVE')));

?>