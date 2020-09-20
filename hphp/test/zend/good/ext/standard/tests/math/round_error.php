<?hh
/* Prototype  : float round  ( float $val  [, int $precision  ] )
 * Description: Returns the rounded value of val  to specified precision (number of digits
 * after the decimal point)
 * Source code: ext/standard/math.c
 */

/*
 * Pass incorrect number of arguments to round() to test behaviour
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing round() : error conditions ***\n";

echo "\n-- Wrong nmumber of arguments --\n";
try { var_dump(round()); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
var_dump(round(500, 10, 1));
echo "===Done===";
}
