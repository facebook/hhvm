<?hh
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : basic functionality - using string format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%s";
$format2 = "%s %s";
$format3 = "%s %s %s";
$arg1 = vec["one"];
$arg2 = vec["one","two"];
$arg3 = vec["one","two","three"];


var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format3,$arg3) );

echo "Done";
}
