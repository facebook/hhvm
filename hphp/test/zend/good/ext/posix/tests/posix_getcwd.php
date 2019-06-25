<?hh
<<__EntryPoint>> function main(): void {
var_dump(posix_getcwd());
try { var_dump(posix_getcwd(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
