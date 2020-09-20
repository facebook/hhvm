<?hh
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is 
                specified, break the string down into chunks each 
                split_length characters long. 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing str_split() : error conditions ***\n";

// Zero arguments
echo "-- Testing str_split() function with Zero arguments --\n";
try { var_dump( str_split() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test str_split with one more than the expected number of arguments
echo "-- Testing str_split() function with more than expected no. of arguments --\n";
$str = 'This is error testcase';
$split_length = 4;
$extra_arg = 10;
try { var_dump( str_split( $str, $split_length, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
