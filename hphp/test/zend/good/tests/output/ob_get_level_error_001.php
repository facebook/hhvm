<?hh
/* Prototype  : proto int ob_get_level(void)
 * Description: Return the nesting level of the output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ob_get_level() : error conditions ***\n";

// One argument
echo "\n-- Testing ob_get_level() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( ob_get_level($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
