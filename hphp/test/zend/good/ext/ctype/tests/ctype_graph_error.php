<?hh
/* Prototype  : bool ctype_graph(mixed $c)
 * Description: Checks for any printable character(s) except space 
 * Source code: ext/ctype/ctype.c
 */

/*
 * Pass an incorrect number of arguments to ctype_graph() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing ctype_graph() : error conditions ***\n";

// Zero arguments
echo "\n-- Testing ctype_graph() function with Zero arguments --\n";
try { var_dump( ctype_graph() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

//Test ctype_graph with one more than the expected number of arguments
echo "\n-- Testing ctype_graph() function with more than expected no. of arguments --\n";
$c = 1;
$extra_arg = 10;
try { var_dump( ctype_graph($c, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===DONE===\n";
}
