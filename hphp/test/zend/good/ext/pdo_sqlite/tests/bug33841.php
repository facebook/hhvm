<?hh
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
<<__EntryPoint>> function main(): void {
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');
$db->exec('CREATE TABLE test (text)');

$stmt = $db->prepare("INSERT INTO test VALUES ( :text )");
$name = 'test1';
$stmt->bindValue(':text', $name);
var_dump($stmt->execute(), $stmt->rowCount());

$stmt = $db->prepare("UPDATE test SET text = :text ");
$name = 'test2';
$stmt->bindValue(':text', $name);
var_dump($stmt->execute(), $stmt->rowCount());
}
