<?hh
<<__EntryPoint>> function main(): void {
var_dump(filter_list());
try { var_dump(filter_list(vec[])); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done\n";
}
