<?hh

/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sscanf() : basic functionality -using exponential format ***\n";

$str = "10.12345 10.12345E3 10.12345e3 -10.12345e4" ;
$format1 = "%e %e %e %e";
$format2 = "%E %E %E %E";

echo "\n-- Try sccanf() WITHOUT optional args --\n";
// extract details using short format
list($arg1, $arg2, $arg3, $arg4) = sscanf($str, $format1);
var_dump($arg1, $arg2, $arg3, $arg4);
list($arg1, $arg2, $arg3, $arg4) = sscanf($str, $format2);
var_dump($arg1, $arg2, $arg3, $arg4);

echo "===DONE===\n";
}
