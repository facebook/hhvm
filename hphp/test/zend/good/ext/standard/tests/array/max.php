<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(max()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(max(1));
var_dump(max(vec[]));
var_dump(max(new stdClass));
var_dump(max(2,1,2));
var_dump(max(2.1,2.11,2.09));
var_dump(max("", "t", "b"));
var_dump(max(false, true, false));
var_dump(max(true, false, true));

echo "Done\n";
}
