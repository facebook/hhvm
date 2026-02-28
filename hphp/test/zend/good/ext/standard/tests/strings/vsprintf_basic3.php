<?hh
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : basic functionality - using float format ***\n";

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

var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format11,$arg1) );

var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format22,$arg2) );

var_dump( vsprintf($format3,$arg3) );
var_dump( vsprintf($format33,$arg3) );

echo "Done";
}
