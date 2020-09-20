<?hh
/* Prototype  : array gethostbynamel  ( string $hostname  )
 * Description: Get a list of IPv4 addresses corresponding to a given Internet host name 
 * Source code: ext/standard/dns.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing gethostbynamel() : basic functionality ***\n";
var_dump(gethostbynamel("localhost"));
echo "===DONE===\n";
}
