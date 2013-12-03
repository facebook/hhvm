<?php
include __DIR__ . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();

$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);

$createSql = "CREATE TABLE `bug53551` (
  `count` bigint(20) unsigned NOT NULL DEFAULT '0'
)";

$db->exec('drop table if exists bug53551');
$db->exec($createSql);
$db->exec("insert into bug53551 set `count` = 1 ");
$db->exec("SET sql_mode = 'Traditional'");
$sql = 'UPDATE bug53551 SET `count` = :count';
$stmt = $db->prepare($sql);

$values = array (
    'count' => NULL,
);

echo "1\n";
$stmt->execute($values);
var_dump($stmt->errorInfo());

echo "2\n";
$stmt->execute($values);
var_dump($stmt->errorInfo());

echo "\ndone\n";

?>
<?php
include __DIR__ . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec('DROP TABLE IF EXISTS bug53551');
?>