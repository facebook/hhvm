<?hh
<<__EntryPoint>> function main(): void {
echo "*** Test by calling method or function with more than expected arguments ***\n";

// test without any error
try { var_dump(posix_errno('bar')); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
