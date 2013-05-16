<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();

$db->query('DROP TABLE IF EXISTS test');
$db->query('create table `test`( `id` int )');

$handle1 = $db->prepare('insert into test(id) values(1);
                          select * from test where id = ?;
                          update test set id = 2 where id = ?;');

$handle1->bindValue('1', '1');
$handle1->bindValue('2', '1');

$handle1->execute();
$i = 1;
print("Handle 1:\n");
do {
	print('Rowset ' . $i++ . "\n");
	if ($handle1->columnCount() > 0)
		print("Results detected\n");
} while($handle1->nextRowset());

$handle2 = $db->prepare('select * from test where id = ?;
                           update test set id = 1 where id = ?;');

$handle2->bindValue('1', '2');
$handle2->bindValue('2', '2');

$handle2->execute();

$i = 1;
print("Handle 2:\n");
do {
	print('Rowset ' . $i++ . "\n");
	if ($handle2->columnCount() > 0)
		print("Results detected\n");
} while($handle2->nextRowset());

$handle3 = $db->prepare('update test set id = 2 where id = ?;
                           select * from test where id = ?;');

$handle3->bindValue('1', '1');
$handle3->bindValue('2', '2');

$handle3->execute();

$i = 1;
print("Handle 3:\n");
do {
	print('Rowset ' . $i++ . "\n");
	if ($handle3->columnCount() > 0)
		print("Results detected\n");
} while($handle3->nextRowset());

$handle4 = $db->prepare('insert into test(id) values(3);
                           update test set id = 2 where id = ?;
                           select * from test where id = ?;');

$handle4->bindValue('1', '3');
$handle4->bindValue('2', '2');

$handle4->execute();

$i = 1;
print("Handle 4:\n");
do {
	print('Rowset ' . $i++ . "\n");
	if ($handle1->columnCount() > 0)
		print("Results detected\n");
} while($handle1->nextRowset());

$db->query("DROP TABLE test");
?><?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>