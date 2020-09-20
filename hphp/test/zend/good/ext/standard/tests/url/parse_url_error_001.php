<?hh
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing parse_url() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing parse_url() function with Zero arguments --\n";
try { var_dump( parse_url() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test parse_url with one more than the expected number of arguments
echo "\n-- Testing parse_url() function with more than expected no. of arguments --\n";
$url = 'string_val';
$url_component = 10;
$extra_arg = 10;
try { var_dump( parse_url($url, $url_component, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
