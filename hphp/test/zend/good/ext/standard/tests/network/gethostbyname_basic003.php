<?hh
/* Prototype  : string gethostbyname  ( string $hostname  )
 * Description: Get the IPv4 address corresponding to a given Internet host name 
 * Source code: ext/standard/dns.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing gethostbyname() : basic functionality ***\n";

echo gethostbyname("localhost")."\n";
echo "===DONE===\n";
}
