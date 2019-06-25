<?hh
/* Prototype  : proto int posix_get_last_error(void)
 * Description: Retrieve the error number set by the last posix function which failed. 
 * Source code: ext/posix/posix.c
 * Alias to functions: posix_errno
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_get_last_error() : error conditions ***\n";

// One argument
echo "\n-- Testing posix_get_last_error() function with one argument --\n";
$extra_arg = 10;
try { var_dump( posix_get_last_error($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
