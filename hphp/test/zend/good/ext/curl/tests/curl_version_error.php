<?hh

/* Prototype  : array curl_version  ([ int $age  ] )
 * Description: Returns information about the cURL version.
 * Source code: ext/curl/interface.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing curl_version() : error conditions ***\n";

echo "\n-- Testing curl_version() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( curl_version(1, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
