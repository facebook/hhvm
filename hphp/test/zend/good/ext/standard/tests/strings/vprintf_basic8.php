<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using octal format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%o";
$format2 = "%o %o";
$format3 = "%o %o %o";
$arg1 = vec[021];
$arg2 = vec[021,0347];
$arg3 = vec[021,0347,0567];

$result = vprintf($format1,$arg1);
echo "\n";
var_dump($result);

$result = vprintf($format2,$arg2);
echo "\n";
var_dump($result);

$result = vprintf($format3,$arg3);
echo "\n";
var_dump($result);

echo "===DONE===\n";
}
