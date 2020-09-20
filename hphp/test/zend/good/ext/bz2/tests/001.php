<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(bzopen()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(bzopen("", ""));
var_dump(bzopen("", "r"));
var_dump(bzopen("", "w"));
var_dump(bzopen("", "x"));
var_dump(bzopen("", "rw"));
var_dump(bzopen("no_such_file", "r"));

$fp = fopen(__FILE__,"r");
var_dump(bzopen($fp, "r"));

echo "Done\n";
}
