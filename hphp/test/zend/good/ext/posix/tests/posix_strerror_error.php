<?hh
/* Prototype  : proto string posix_strerror(int errno)
 * Description: Retrieve the system error message associated with the given errno. 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_strerror() : error conditions ***\n";

echo "\n-- Testing posix_strerror() function with Zero arguments --\n";
try { var_dump( posix_strerror() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing posix_strerror() function with more than expected no. of arguments --\n";
$errno = posix_get_last_error();
$extra_arg = 10;
try { var_dump( posix_strerror($errno, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing posix_strerror() function with invalid error number --\n";
$errno = -999;
echo gettype( posix_strerror($errno) )."\n";

echo "Done";
}
