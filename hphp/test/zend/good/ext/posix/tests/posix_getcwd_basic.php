<?hh <<__EntryPoint>> function main(): void {
echo "Basic test of POSIX posix_getcwd function\n";
var_dump(posix_getcwd());
try { var_dump(posix_getcwd(1)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
