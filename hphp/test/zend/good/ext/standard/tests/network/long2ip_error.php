<?hh
/* Prototype  : string long2ip(int proper_address)
 * Description: Converts an (IPv4) Internet network address into a string in Internet standard dotted format 
 * Source code: ext/standard/basic_functions.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing long2ip() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing long2ip() function with Zero arguments --\n";
try { var_dump( long2ip() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test long2ip with one more than the expected number of arguments
echo "\n-- Testing long2ip() function with more than expected no. of arguments --\n";
$proper_address = 10;
$extra_arg = 10;
try { var_dump( long2ip($proper_address, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
