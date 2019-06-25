<?hh
/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/

/*
 * Testing sscanf() : basic functionality
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sscanf() : basic functionality - using string format ***\n";

$str = "Part: Widget Serial Number: 1234789 Stock: 25";
$format = "Part: %s Serial Number: %s Stock: %s";

echo "\n-- Try sccanf() WITHOUT optional args --\n";
// extract details using short format
list($part, $number, $stock) = sscanf($str, $format);
var_dump($part, $number, $stock);

echo "===DONE===\n";
}
