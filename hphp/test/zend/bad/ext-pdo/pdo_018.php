<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

class TestBase implements Serializable
{
	public    $BasePub = 'Public';
	protected $BasePro = 'Protected';
	private   $BasePri = 'Private';
	
	function serialize()
	{
		$serialized = array();
		foreach($this as $prop => $val) {
			$serialized[$prop] = $val;
		}
		$serialized = serialize($serialized);
		echo __METHOD__ . "() = '$serialized'\n";
		return $serialized;
	}
	
	function unserialize($serialized)
	{
		echo __METHOD__ . "($serialized)\n";
		foreach(unserialize($serialized) as $prop => $val) {
			$this->$prop = '#'.$val;
		}
		return true;
	}
}

class TestDerived extends TestBase
{
	public    $BasePub    = 'DerivedPublic';
	protected $BasePro    = 'DerivdeProtected';
	public    $DerivedPub = 'Public';
	protected $DerivedPro = 'Protected';
	private   $DerivedPri = 'Private';

	function serialize()
	{
		echo __METHOD__ . "()\n";
		return TestBase::serialize();
	}
	
	function unserialize($serialized)
	{
		echo __METHOD__ . "()\n";
		return TestBase::unserialize($serialized);
	}
}

class TestLeaf extends TestDerived
{
}

$db->exec('CREATE TABLE classtypes(id int NOT NULL PRIMARY KEY, name VARCHAR(20) NOT NULL UNIQUE)');
$db->exec('INSERT INTO classtypes VALUES(0, \'stdClass\')'); 
$db->exec('INSERT INTO classtypes VALUES(1, \'TestBase\')'); 
$db->exec('INSERT INTO classtypes VALUES(2, \'TestDerived\')'); 
$db->exec('CREATE TABLE test(id int NOT NULL PRIMARY KEY, classtype int, val VARCHAR(255))');

$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

var_dump($db->query('SELECT COUNT(*) FROM classtypes')->fetchColumn());
var_dump($db->query('SELECT id, name FROM classtypes ORDER by id')->fetchAll(PDO::FETCH_COLUMN|PDO::FETCH_UNIQUE));

$objs = array();
$objs[0] = new stdClass;
$objs[1] = new TestBase;
$objs[2] = new TestDerived;
$objs[3] = new TestLeaf;

$stmt = $db->prepare('SELECT id FROM classtypes WHERE name=:cname');
$stmt->bindParam(':cname', $cname);

$ctypes = array();

foreach($objs as $obj)
{
	$cname = get_class($obj);
	$ctype = NULL; /* set default for non stored class name */
	$stmt->execute();
	$stmt->bindColumn('id', $ctype);
	$stmt->fetch(PDO::FETCH_BOUND);
	$ctypes[$cname] = $ctype;
}

echo "===TYPES===\n";
var_dump($ctypes);

unset($stmt);

echo "===INSERT===\n";
$stmt = $db->prepare('INSERT INTO test VALUES(:id, :classtype, :val)');
$stmt->bindParam(':id', $idx);
$stmt->bindParam(':classtype', $ctype);
$stmt->bindParam(':val', $val);

foreach($objs as $idx => $obj)
{
	$ctype = $ctypes[get_class($obj)];
	if (method_exists($obj, 'serialize'))
	{
		$val = $obj->serialize();
	}
	else
	{
		$val = '';
	}
	$stmt->execute();	
}

unset($stmt);

echo "===DATA===\n";
$res = $db->query('SELECT test.val FROM test')->fetchAll(PDO::FETCH_COLUMN);

// For Oracle map NULL to empty string so the test doesn't diff
if ($db->getAttribute(PDO::ATTR_DRIVER_NAME) == 'oci' && $res[0] === null) {
    $res[0] = "";
}
var_dump($res);

echo "===FAILURE===\n";
try
{
	$db->query('SELECT classtypes.name AS name, test.val AS val FROM test LEFT JOIN classtypes ON test.classtype=classtypes.id')->fetchAll(PDO::FETCH_CLASS|PDO::FETCH_CLASSTYPE|PDO::FETCH_SERIALIZE, 'TestLeaf', array());
}
catch (PDOException $e)
{
	echo 'Exception:';
	echo $e->getMessage()."\n";
}

echo "===COUNT===\n";
var_dump($db->query('SELECT COUNT(*) FROM test LEFT JOIN classtypes ON test.classtype=classtypes.id WHERE (classtypes.id IS NULL OR classtypes.id > 0)')->fetchColumn());

echo "===DATABASE===\n";
$stmt = $db->prepare('SELECT classtypes.name AS name, test.val AS val FROM test LEFT JOIN classtypes ON test.classtype=classtypes.id WHERE (classtypes.id IS NULL OR classtypes.id > 0)');

$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

echo "===FETCHCLASS===\n";
$stmt->execute();
var_dump($stmt->fetchAll(PDO::FETCH_CLASS|PDO::FETCH_CLASSTYPE|PDO::FETCH_SERIALIZE, 'TestLeaf'));


?>