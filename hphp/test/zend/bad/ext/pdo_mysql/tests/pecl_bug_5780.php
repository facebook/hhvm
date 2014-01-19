<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__). '/common.phpt');

$db->exec("CREATE TABLE test (login varchar(32) NOT NULL, data varchar(64) NOT NULL)");
$db->exec("CREATE TABLE test2 (login varchar(32) NOT NULL, password varchar(64) NOT NULL)");
$db->exec("INSERT INTO test2 (login, password) VALUES ('testing', 'testing')");
$db->exec("INSERT INTO test2 (login, password) VALUES ('test2', 'testpw2')");

$logstmt = $db->prepare('INSERT INTO test (login, data) VALUES (:var1, :var2)');
$authstmt = $db->prepare('SELECT * FROM test2 WHERE login = :varlog AND password = :varpass');
$authstmt->execute(array(':varlog' => 'testing', ':varpass' => 'testing'));
var_dump($authstmt->fetch(PDO::FETCH_NUM));
@var_dump($logstmt->execute(array(':var1' => 'test1', ':var2' => 'test2')));
$info = $logstmt->errorInfo();
unset($info[2]);
var_dump($info);
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec('DROP TABLE IF EXISTS test');
$db->exec('DROP TABLE IF EXISTS test2');
?>