<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
$db->exec('CREATE TABLE foo (id INTEGER, bar STRING)');
$db->exec("INSERT INTO foo (id, bar) VALUES (1, 'This is a test')");

$stmt = $db->prepare('SELECT foo FROM bar');

var_dump($stmt);
}
