<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using bool format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%b";
$format2 = "%b %b";
$format3 = "%b %b %b";
$arg1 = vec[TRUE];
$arg2 = vec[TRUE,FALSE];
$arg3 = vec[TRUE,FALSE,TRUE];

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
