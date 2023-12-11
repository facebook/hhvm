<?hh
/* Prototype  : string vprintf(string $format , array $args)
 * Description: Output a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 *  Testing vprintf() : basic functionality - using integer format
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vprintf() : basic functionality - using integer format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%d";
$format2 = "%d %d";
$format3 = "%d %d %d";
$arg1 = vec[111];
$arg2 = vec[111,222];
$arg3 = vec[111,222,333];

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
