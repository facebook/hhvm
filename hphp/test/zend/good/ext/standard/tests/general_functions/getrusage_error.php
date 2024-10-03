<?hh
/* Prototype  :  array getrusage  ([ int $who  ] )
 * Description: Gets the current resource usages
 * Source code: ext/standard/microtime.c
 * Alias to functions:
 */
/*
 * Pass an incorrect number of arguments to getrusage() to test behaviour
 */
class classA { function __toString() :mixed{ return "ClassAObject"; } }
<<__EntryPoint>> function main(): void {
echo "*** Testing getrusage() : error conditions ***\n";

echo "\n-- Testing getrusage() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { $dat = getrusage(1, $extra_arg); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";
}
