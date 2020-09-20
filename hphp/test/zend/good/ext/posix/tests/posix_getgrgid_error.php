<?hh
/* Prototype  : proto array posix_getgrgid(long gid)
 * Description: Group database access (POSIX.1, 9.2.1) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing posix_getgrgid() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing posix_getgrgid() function with Zero arguments --\n";
try { var_dump( posix_getgrgid() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test posix_getgrgid with one more than the expected number of arguments
echo "\n-- Testing posix_getgrgid() function with more than expected no. of arguments --\n";

$extra_arg = 10;
$gid = 0;
try { var_dump( posix_getgrgid($gid, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing posix_getgrgid() function with a negative group id --\n";
$gid = -999;
var_dump( posix_getgrgid($gid));

echo "Done";
}
