<?hh
/* Prototype  : bool closelog(void)
 * Description: Close connection to system logger 
 * Source code: ext/standard/syslog.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing closelog() : basic functionality ***\n";

// Zero arguments
echo "\n-- Testing closelog() function with Zero arguments --\n";
var_dump( closelog() );
echo "===DONE===\n";
}
