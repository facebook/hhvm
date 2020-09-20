<?hh
/* Prototype  : proto array posix_times(void)
 * Description: Get process times (POSIX.1, 4.5.2) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_times() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_times() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( posix_times($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
