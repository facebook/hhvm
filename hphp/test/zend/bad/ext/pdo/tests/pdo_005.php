<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), val2 VARCHAR(10))');
$db->exec("INSERT INTO test VALUES(1, 'A', 'AA')");
$db->exec("INSERT INTO test VALUES(2, 'B', 'BB')");
$db->exec("INSERT INTO test VALUES(3, 'C', 'CC')");

$stmt = $db->prepare('SELECT id, val, val2 from test');

class TestBase
{
	public $id;
	protected $val;
	private $val2;
}

class TestDerived extends TestBase
{
	protected $row;

	public function __construct(&$row)
	{
		echo __METHOD__ . "($row,{$this->id})\n";
		$this->row = $row++;
	}
}

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_CLASS));

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_CLASS, 'TestBase'));

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_CLASS, 'TestDerived', array(0)));

?>