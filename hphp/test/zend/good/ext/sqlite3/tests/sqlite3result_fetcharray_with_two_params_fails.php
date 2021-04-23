<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
$db->exec('CREATE TABLE foo (bar STRING)');
$db->exec("INSERT INTO foo (bar) VALUES ('This is a test')");
$db->exec("INSERT INTO foo (bar) VALUES ('This is another test')");

$result = $db->query('SELECT bar FROM foo');
try { var_dump($result->fetcharray(1,2)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
