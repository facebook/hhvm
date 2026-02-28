<?hh
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : basic functionality - using bool format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%b";
$format2 = "%b %b";
$format3 = "%b %b %b";
$arg1 = vec[TRUE];
$arg2 = vec[TRUE,FALSE];
$arg3 = vec[TRUE,FALSE,TRUE];

var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format3,$arg3) );

echo "Done";
}
