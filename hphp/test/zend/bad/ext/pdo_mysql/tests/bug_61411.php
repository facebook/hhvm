<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

$attr	= getenv('PDOTEST_ATTR');
if (!$attr) {
	$attr = array();
} else {
	$attr = unserialize($attr);
}
$attr[PDO::ATTR_PERSISTENT] = true;
$attr[PDO::ATTR_EMULATE_PREPARES] = false;
putenv('PDOTEST_ATTR='.serialize($attr));

$db = MySQLPDOTest::factory();

$stmt = $db->prepare("SELECT 1");
$stmt->execute();

foreach ($stmt as $line) {
	var_dump($line);
}

print "done!";
?>