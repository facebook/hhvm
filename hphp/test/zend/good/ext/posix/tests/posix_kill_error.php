<?hh
/* Prototype  : proto bool posix_kill(int pid, int sig)
 * Description: Send a signal to a process (POSIX.1, 3.3.2) 
 * Source code: ext/posix/posix.c
 * Alias to functions: 
 */

<<__EntryPoint>> function main(): void {
echo "*** Testing posix_kill() : error conditions ***\n";


echo "\n-- Testing posix_kill() function with more than expected no. of arguments --\n";
$pid = posix_getpid();
$sig = 9;
$extra_arg = 10;
try { var_dump( posix_kill($pid, $sig, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing posix_kill() function with less than expected no. of arguments --\n";
$pid = posix_getpid();
try { var_dump( posix_kill($pid) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing posix_kill() function with invalid signal --\n";
$pid = posix_getpid();
$sig = 999;
var_dump( posix_kill($pid, 999) );

echo "\n-- Testing posix_kill() function with negative pid --\n";
$pid = -999;
$sig = 9;
var_dump( posix_kill($pid, 999) );

echo "Done";
}
