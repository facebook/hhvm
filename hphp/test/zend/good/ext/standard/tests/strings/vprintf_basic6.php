<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using exponential format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%e";
$format2 = "%e %e";
$format3 = "%e %e %e";
$arg1 = vec[1000];
$arg2 = vec[1000,2000];
$arg3 = vec[1000,2000,3000];

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
