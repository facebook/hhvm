<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
echo 'Testing SQLite3 querySingle without parameters' . PHP_EOL;
try { $db->querySingle(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 querySingle with one array parameter' . PHP_EOL;
try { $db->querySingle(varray[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 qeurySingle with empty string parameter' . PHP_EOL;
var_dump($db->querySingle(''));

echo "Done";
}
