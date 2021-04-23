<?hh
<<__EntryPoint>> function test(): void {
$db = new SQLite3(':memory:');

var_dump($db->loadExtension('myext.txt'));
var_dump($db->close());

echo "Done\n";
}
