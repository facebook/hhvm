<?hh
/* Prototype  : proto bool ob_clean(void)
 * Description: Clean (delete) the current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ob_clean() : error conditions ***\n";

// One argument
echo "\n-- Testing ob_clean() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( ob_clean($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
