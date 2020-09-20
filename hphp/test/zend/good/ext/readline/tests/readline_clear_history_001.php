<?hh
<<__EntryPoint>> function main(): void {
var_dump(readline_clear_history());
try { var_dump(readline_clear_history(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
