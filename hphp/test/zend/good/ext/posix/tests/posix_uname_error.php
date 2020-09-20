<?hh
/* Prototype  : proto array posix_uname(void)
 * Description: Get system name (POSIX.1, 4.4.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_uname() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_uname() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( posix_uname($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
