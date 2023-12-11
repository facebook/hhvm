<?hh <<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
echo 'Testing SQLite3 querysingle without parameters' . PHP_EOL;
try { $db->querysingle(); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 querysingle with one array parameter' . PHP_EOL;
try { $db->querysingle(vec[]); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo 'Testing SQLite3 querysingle with empty string parameter' . PHP_EOL;
var_dump($db->querysingle(''));

echo "Done";
}
