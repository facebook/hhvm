<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using char format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%c";
$format2 = "%c %c";
$format3 = "%c %c %c";
$arg1 = vec[65];
$arg2 = vec[65,66];
$arg3 = vec[65,66,67];

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
