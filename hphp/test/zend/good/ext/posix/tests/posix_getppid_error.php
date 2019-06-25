<?hh
/* Prototype  : proto int posix_getppid(void)
 * Description: Get the parent process id (POSIX.1, 4.1.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_getppid() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_getppid() function with one argument --\n";
$extra_arg = 10;
try { var_dump( posix_getppid($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
