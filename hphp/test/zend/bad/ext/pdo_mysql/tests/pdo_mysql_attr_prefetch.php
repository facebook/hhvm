<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	var_dump($db->getAttribute(PDO::ATTR_PREFETCH));
	var_dump($db->setAttribute(PDO::ATTR_PREFETCH, true));
	print "done!";