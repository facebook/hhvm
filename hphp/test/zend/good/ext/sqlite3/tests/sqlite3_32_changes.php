<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
//$db = new SQLite3('mysqlitedb.db');
$db->exec('CREATE TABLE pageView(id INTEGER PRIMARY KEY, page CHAR(256), access INTEGER(10))');
$db->exec('INSERT INTO pageView (page, access) VALUES (\'test\', \'000000\')');
try { echo $db->changes("dummy"); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
