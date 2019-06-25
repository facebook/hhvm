<?hh
/* Prototype  : proto bool ob_flush(void)
 * Description: Flush (send) contents of the output buffer. The last buffer content is sent to next buffer 
 * Source code: main/output.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ob_flush() : error conditions ***\n";

// One argument
echo "\n-- Testing ob_flush() function with one argument --\n";
$extra_arg = 10;;
try { var_dump( ob_flush($extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "Done";
}
