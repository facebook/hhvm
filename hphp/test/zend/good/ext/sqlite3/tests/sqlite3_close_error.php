<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
echo 'Testing SQLite3 close with one parameter' . PHP_EOL;
try { $db->close('parameter'); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
