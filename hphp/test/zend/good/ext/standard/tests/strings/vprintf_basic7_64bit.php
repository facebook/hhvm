<?hh
/* Prototype  : string vprintf(string $format , aaray $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using unsigned format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%u";
$format2 = "%u %u";
$format3 = "%u %u %u";
$arg1 = vec[-1111];
$arg2 = vec[-1111,-1234567];
$arg3 = vec[-1111,-1234567,-2345432];

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
