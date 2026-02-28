<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using float format ***\n";

// Initialise all required variables

$format = "format";
$format1 = "%f";
$format2 = "%f %f";
$format3 = "%f %f %f";

$format11 = "%F";
$format22 = "%F %F";
$format33 = "%F %F %F";
$arg1 = vec[11.11];
$arg2 = vec[11.11,22.22];
$arg3 = vec[11.11,22.22,33.33];

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

$result = vprintf($format3,$arg3);
echo "\n";
var_dump($result);

$result = vprintf($format33,$arg3);
echo "\n";
var_dump($result);

echo "===DONE===\n";
}
