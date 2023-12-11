<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
echo 'Testing SQLite3 query without parameters' . PHP_EOL;
try { $db->query(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 query with one array parameter' . PHP_EOL;
try { $db->query(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 qeury with empty string parameter' . PHP_EOL;
var_dump($db->query(''));

echo "Done";
}
