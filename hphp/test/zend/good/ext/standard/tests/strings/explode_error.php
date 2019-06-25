<?hh

/* Prototype  : array explode  ( string $delimiter  , string $string  [, int $limit  ] )
 * Description: Split a string by string.
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing explode() : error conditions ***\n";

echo "\n-- Testing explode() function with no arguments --\n";
try { var_dump( explode() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing explode() function with more than expected no. of arguments --\n";
$delimiter = " ";
$string = "piece1 piece2 piece3 piece4 piece5 piece6";
$limit = 5;
$extra_arg = 10;
try { var_dump( explode($delimiter, $string, $limit, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "===Done===";
}
