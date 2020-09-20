<?hh
/* Prototype  : bool syslog(int priority, string message)
 * Description: Generate a system log message 
 * Source code: ext/standard/syslog.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing syslog() : error conditions ***\n";


//Test syslog with one more than the expected number of arguments
echo "\n-- Testing syslog() function with more than expected no. of arguments --\n";
$priority = 10;
$message = 'string_val';
$extra_arg = 10;
try { var_dump( syslog($priority, $message, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// Testing syslog with one less than the expected number of arguments
echo "\n-- Testing syslog() function with less than expected no. of arguments --\n";
$priority = 10;
try { var_dump( syslog($priority) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
