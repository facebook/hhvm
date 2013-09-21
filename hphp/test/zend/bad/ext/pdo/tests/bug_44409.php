<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec("CREATE TABLE test (dat varchar(100))");
$db->exec("INSERT INTO test (dat) VALUES ('Data from DB')");

class bug44409 implements Serializable
{
	public function __construct() 
	{
		printf("Method called: %s()\n", __METHOD__); 
	} 	

	public function serialize()
	{
		return "any data from serizalize()"; 
	}
	
	public function unserialize($dat)
	{
		printf("Method called: %s(%s)\n", __METHOD__, var_export($dat, true));
	}
}

$stmt = $db->query("SELECT * FROM test");

print_r($stmt->fetchAll(PDO::FETCH_CLASS|PDO::FETCH_SERIALIZE, "bug44409"));

?>