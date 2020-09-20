<?hh
/* Prototype  : bool closelog(void)
 * Description: Close connection to system logger 
 * Source code: ext/standard/syslog.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing closelog() : error conditions ***\n";

// One argument
echo "\n-- Testing closelog() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( closelog($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
