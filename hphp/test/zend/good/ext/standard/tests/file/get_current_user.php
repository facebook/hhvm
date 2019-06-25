<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(get_current_user("blah")); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(get_current_user());

echo "Done\n";
}
