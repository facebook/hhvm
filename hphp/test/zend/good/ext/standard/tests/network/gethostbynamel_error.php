<?hh
/* Prototype  : proto array gethostbynamel(string hostname)
 * Description: Return a list of IP addresses that a given hostname resolves to. 
 * Source code: ext/standard/dns.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing gethostbynamel() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing gethostbynamel() function with Zero arguments --\n";
try { var_dump( gethostbynamel() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test gethostbynamel with one more than the expected number of arguments
echo "\n-- Testing gethostbynamel() function with more than expected no. of arguments --\n";
$hostname = 'string_val';
$extra_arg = 10;
try { var_dump( gethostbynamel($hostname, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done";
}
