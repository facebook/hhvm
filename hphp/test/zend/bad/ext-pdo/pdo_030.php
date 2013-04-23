<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/'); 
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';

$data = array(
    array('10', 'Abc', 'zxy'),
    array('20', 'Def', 'wvu'),
    array('30', 'Ghi', 'tsr'),
);

class PDOStatementX extends PDOStatement
{
    public $dbh;
    
    protected function __construct($dbh)
    {
    	$this->dbh = $dbh;
    	$this->setFetchMode(PDO::FETCH_ASSOC);
    	echo __METHOD__ . "()\n";
    }
    
    function __destruct()
    {
    	echo __METHOD__ . "()\n";
    }
    
    function execute($params = array())
    {
    	echo __METHOD__ . "()\n";
		parent::execute();    	
    }
}

class PDODatabase extends PDO
{
    function __destruct()
    {
    	echo __METHOD__ . "()\n";
    }
    
    function query($sql)
    {
    	echo __METHOD__ . "()\n";
    	return parent::query($sql);
    }
}

$db = PDOTest::factory('PDODatabase');
var_dump(get_class($db));

$db->exec('CREATE TABLE test(id INT NOT NULL PRIMARY KEY, val VARCHAR(10), val2 VARCHAR(16))');

$stmt = $db->prepare("INSERT INTO test VALUES(?, ?, ?)");
var_dump(get_class($stmt));
foreach ($data as $row) {
    $stmt->execute($row);
}

unset($stmt);

echo "===QUERY===\n";

var_dump($db->getAttribute(PDO::ATTR_STATEMENT_CLASS));
$db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('PDOStatementx', array($db)));
var_dump($db->getAttribute(PDO::ATTR_STATEMENT_CLASS));
$stmt = $db->query('SELECT * FROM test');
var_dump(get_class($stmt));
var_dump(get_class($stmt->dbh));

echo "===FOREACH===\n";

foreach($stmt as $obj) {
	var_dump($obj);
}

echo "===DONE===\n";
exit(0);
?>