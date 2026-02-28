<?hh
<<__EntryPoint>> function main(): void {
$db = new SQLite3(':memory:');
try { $db->exec(vec['a','b','c'], 20090509); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
