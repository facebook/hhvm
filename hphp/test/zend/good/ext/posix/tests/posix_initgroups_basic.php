<?hh <<__EntryPoint>> function main(): void {
echo "Basic test of POSIX posix_initgroups function\n";
try { var_dump(posix_initgroups('foo', 'bar')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(posix_initgroups(NULL, NULL)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE====";
}
