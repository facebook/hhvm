<?hh
/* Prototype  : proto string base64_encode(string str)
 * Description: Encodes string using MIME base64 algorithm 
 * Source code: ext/standard/base64.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing base64_encode() : error conditions - wrong number of args ***\n";

// Zero arguments
echo "\n-- Testing base64_encode() function with Zero arguments --\n";
try { var_dump( base64_encode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test base64_encode with one more than the expected number of arguments
echo "\n-- Testing base64_encode() function with more than expected no. of arguments --\n";
$str = 'string_val';
$extra_arg = 10;
try { var_dump( base64_encode($str, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
