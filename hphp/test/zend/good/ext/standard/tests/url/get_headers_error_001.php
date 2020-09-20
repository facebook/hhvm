<?hh
/* Prototype  : proto array get_headers(string url[, int format])
 * Description: Fetches all the headers sent by the server in response to a HTTP request
 * Source code: ext/standard/url.c
 * Alias to functions:
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing get_headers() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing get_headers() function with Zero arguments --\n";
try { var_dump( get_headers() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test get_headers with one more than the expected number of arguments
echo "\n-- Testing get_headers() function with more than expected no. of arguments --\n";
$url       = 'string_val';
$format    = 1;
$extra_arg = 10;
try { var_dump( get_headers($url, $format, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
