<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, val VARCHAR(10), grp VARCHAR(10))');
$db->exec('INSERT INTO test VALUES(1, \'A\', \'Group1\')'); 
$db->exec('INSERT INTO test VALUES(2, \'B\', \'Group2\')'); 
$SELECT = 'SELECT val, grp FROM test';

class Test
{
	function __construct($name = 'N/A')
	{
		echo __METHOD__ . "($name)\n";
	}
}

$stmt = $db->query($SELECT, PDO::FETCH_CLASS, 'Test', array('WOW'));

$it = new IteratorIterator($stmt); /* check if we can convert that thing */

/*** HINT: If YOU plan to do so remember not to call rewind() -> see below ***/

foreach($it as $data)
{
	var_dump($data);
}

$it->next();              /* must be allowed */
var_dump($it->current()); /* must return NULL */
var_dump($it->valid());   /* must return false */

class PDOStatementAggregate extends PDOStatement implements IteratorAggregate
{
	private function __construct()
	{
		echo __METHOD__ . "\n";
		$this->setFetchMode(PDO::FETCH_NUM);   
		/* default fetch mode is BOTH, so we see if the ctor can overwrite that */
	}

	function getIterator()
	{
		echo __METHOD__ . "\n";
		$this->execute();
		return new IteratorIterator($this, 'PDOStatement');
	}
}

$stmt = $db->prepare($SELECT, array(PDO::ATTR_STATEMENT_CLASS=>array('PDOStatementAggregate')));

foreach($stmt as $data)
{
	var_dump($data);
}

?>