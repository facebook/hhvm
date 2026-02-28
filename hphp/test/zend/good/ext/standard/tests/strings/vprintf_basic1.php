<?hh
/* Prototype  : int vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using string format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%s";
$format2 = "%s %s";
$format3 = "%s %s %s";
$arg1 = vec["one"];
$arg2 = vec["one","two"];
$arg3 = vec["one","two","three"];


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
