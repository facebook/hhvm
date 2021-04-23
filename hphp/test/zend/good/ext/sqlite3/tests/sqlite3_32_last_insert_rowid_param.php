<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');

echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "Inserting data\n";
var_dump($db->exec('INSERT INTO test (time, id) VALUES(2, 1)'));

echo "Request last inserted id\n";
try { $db->lastinsertrowid(""); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Closing database\n";
var_dump($db->close());
echo "Done";
}
