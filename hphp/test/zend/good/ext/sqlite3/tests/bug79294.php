<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
$db->exec("CREATE TABLE foo (bar INT)");
$db->exec("INSERT INTO foo VALUES (1)");

$stmt = $db->prepare("SELECT * FROM foo");
$res = $stmt->execute();
var_dump($res->fetcharray() !== false);
var_dump($res->columntype(0));
var_dump($res->fetcharray() !== false);
var_dump($res->columntype(0));
$stmt->reset();
var_dump($res->fetcharray() !== false);
var_dump($res->columntype(0));
$res->reset();
var_dump($res->fetcharray() !== false);
var_dump($res->columntype(0));
}
