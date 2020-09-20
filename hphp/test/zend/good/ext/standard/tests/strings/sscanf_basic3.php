<?hh

/* Prototype  : mixed sscanf  ( string $str  , string $format  [, mixed &$...  ] )
 * Description: Parses input from a string according to a format
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing sscanf() : basic functionality -- using float format ***\n";

$str = "Part: Widget Length: 111.53 Width: 22.345 Depth: 12.4";
$format = "Part: %s Length: %f Width: %f Depth: %f";

echo "\n-- Try sccanf() WITHOUT optional args --\n";
// extract details using short format
list($part, $length, $width, $depth) = sscanf($str, $format);
var_dump($part, $length, $width, $depth);

echo "===DONE===\n";
}
