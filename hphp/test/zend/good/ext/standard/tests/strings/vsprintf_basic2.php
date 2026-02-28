<?hh
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

/*
 *  Testing vsprintf() : basic functionality - using integer format
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : basic functionality - using integer format ***\n";

// Initialise all required variables
$format = "format";
$format1 = "%d";
$format2 = "%d %d";
$format3 = "%d %d %d";
$arg1 = vec[111];
$arg2 = vec[111,222];
$arg3 = vec[111,222,333];

var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format3,$arg3) );

echo "Done";
}
