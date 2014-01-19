<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

$dsn = MySQLPDOTest::getDSN();
$db = new PDO($dsn, PDO_MYSQL_TEST_USER, PDO_MYSQL_TEST_PASS, array(PDO::ATTR_PERSISTENT => true));

print "done!";
?>