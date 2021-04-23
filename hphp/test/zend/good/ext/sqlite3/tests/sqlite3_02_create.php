<?hh
<<__EntryPoint>> function test(): void {
$db = new SQLite3(':memory:');

echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "Creating Same Table Again\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "Dropping database\n";
var_dump($db->exec('DROP TABLE test'));

echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
}
