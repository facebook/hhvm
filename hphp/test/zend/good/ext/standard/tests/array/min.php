<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(min()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(min(1));
var_dump(min(varray[]));
var_dump(min(new stdclass));
var_dump(min(2,1,2));
var_dump(min(2.1,2.11,2.09));
var_dump(min("", "t", "b"));
var_dump(min(false, true, false));
var_dump(min(true, false, true));
var_dump(min(1, true, false, true));
var_dump(min(0, true, false, true));

echo "Done\n";
}
