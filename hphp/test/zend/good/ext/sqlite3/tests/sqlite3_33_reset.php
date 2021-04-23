<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');

$db->exec('CREATE TABLE foo (id INTEGER, bar STRING)');
$db->exec("INSERT INTO foo (id, bar) VALUES (1, 'This is a test')");

$stmt = $db->prepare('SELECT bar FROM foo WHERE id=:id');
$stmt->bindvalue(':id', 1, SQLITE3_INTEGER);
try { $stmt->reset("dummy"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
$stmt->reset();

//var_dump($db);
//var_dump($db->close());
echo "Done\n";
}
