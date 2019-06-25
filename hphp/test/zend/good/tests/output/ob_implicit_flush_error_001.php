<?hh
/* Prototype  : proto void ob_implicit_flush([int flag])
 * Description: Turn implicit flush on/off and is equivalent to calling flush() after every output call 
 * Source code: main/output.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ob_implicit_flush() : error conditions ***\n";


//Test ob_implicit_flush with one more than the expected number of arguments
echo "\n-- Testing ob_implicit_flush() function with more than expected no. of arguments --\n";
$flag = 10;
$extra_arg = 10;
try { var_dump( ob_implicit_flush($flag, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
