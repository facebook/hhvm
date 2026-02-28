<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using hexadecimal format ***\n";

// Initialising different format strings
$format = "format";
$format1 = "%x";
$format2 = "%x %x";
$format3 = "%x %x %x";

$format11 = "%X";
$format22 = "%X %X";
$format33 = "%X %X %X";

$arg1 = vec[11];
$arg2 = vec[11,132];
$arg3 = vec[11,132,177];

$result = vprintf($format1,$arg1);
echo "\n";
var_dump($result);
$result = vprintf($format11,$arg1);
echo "\n";
var_dump($result);

$result = vprintf($format2,$arg2);
echo "\n";
var_dump($result);
$result = vprintf($format22,$arg2);
echo "\n";
var_dump($result);

$result = vprintf($format3,$arg3);echo "\n";
var_dump($result);
$result = vprintf($format33,$arg3);
echo "\n";
var_dump($result);

echo "===DONE===\n";
}
