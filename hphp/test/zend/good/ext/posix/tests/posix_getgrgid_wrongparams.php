<?hh <<__EntryPoint>> function main(): void {
$gid = PHP_INT_MAX; // obscene high gid
var_dump(posix_getgrgid($gid));
var_dump(posix_getgrgid(-1));
try { var_dump(posix_getgrgid()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
