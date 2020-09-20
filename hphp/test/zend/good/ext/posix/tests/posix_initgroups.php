<?hh
<<__EntryPoint>> function main(): void {
try { var_dump(posix_initgroups('foo', 'bar')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
try { var_dump(posix_initgroups(NULL, NULL)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
