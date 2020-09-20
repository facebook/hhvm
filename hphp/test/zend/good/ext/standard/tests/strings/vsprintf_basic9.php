<?hh
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing vsprintf() : basic functionality - using hexadecimal format ***\n";

// Initialising different format strings
$format = "format";
$format1 = "%x";
$format2 = "%x %x";
$format3 = "%x %x %x";

$format11 = "%X";
$format22 = "%X %X";
$format33 = "%X %X %X";

$arg1 = varray[11];
$arg2 = varray[11,132];
$arg3 = varray[11,132,177];

var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format11,$arg1) );

var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format22,$arg2) );

var_dump( vsprintf($format3,$arg3) );
var_dump( vsprintf($format33,$arg3) );

echo "Done";
}
