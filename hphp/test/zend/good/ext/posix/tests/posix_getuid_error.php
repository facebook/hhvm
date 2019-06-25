<?hh
/* Prototype  : proto int posix_getuid(void)
 * Description: Get the current user id (POSIX.1, 4.2.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_getuid() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_getuid() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( posix_getuid($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
