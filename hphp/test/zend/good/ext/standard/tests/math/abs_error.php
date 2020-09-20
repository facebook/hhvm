<?hh
/* Prototype  : number abs  ( mixed $number  )
 * Description: Returns the absolute value of number.
 * Source code: ext/standard/math.c
 */

/*
 * Pass incorrect number of arguments to abs() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing abs() : error conditions ***\n";

$arg_0 = 1.0;
$extra_arg = 1;

echo "\nToo many arguments\n";
try { var_dump(abs($arg_0, $extra_arg)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\nToo few arguments\n";
try { var_dump(abs()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
