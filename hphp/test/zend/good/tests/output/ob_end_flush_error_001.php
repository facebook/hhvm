<?hh
/* Prototype  : proto bool ob_end_flush(void)
 * Description: Flush (send) the output buffer, and delete current output buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ob_end_flush() : error conditions ***\n";

// One argument
echo "\n-- Testing ob_end_flush() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( ob_end_flush($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
