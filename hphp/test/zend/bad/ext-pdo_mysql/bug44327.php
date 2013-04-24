<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();

	$stmt = $db->prepare("SELECT 1 AS \"one\""); 
	$stmt->execute(); 
	$row = $stmt->fetch(PDO::FETCH_LAZY); 
	var_dump($row);
	var_dump($row->{0});
	var_dump($row->one); 
	var_dump($row->queryString);

	print "----------------------------------\n";

	@$db->exec("DROP TABLE test");
	$db->exec("CREATE TABLE test (id INT)");
	$db->exec("INSERT INTO test(id) VALUES (1)");
	$stmt = $db->prepare("SELECT id FROM test");
	$stmt->execute();
	$row = $stmt->fetch(PDO::FETCH_LAZY);
	var_dump($row);
	var_dump($row->queryString);
	@$db->exec("DROP TABLE test");

	print "----------------------------------\n";

	$stmt = $db->prepare('foo'); 
	@$stmt->execute();
	$row = $stmt->fetch();
	var_dump($row->queryString);
	
?>