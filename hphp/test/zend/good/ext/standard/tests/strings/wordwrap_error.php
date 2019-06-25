<?hh
/* Prototype  : string wordwrap ( string $str [, int $width [, string $break [, bool $cut]]] )
 * Description: Wraps buffer to selected number of characters using string break char
 * Source code: ext/standard/string.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing wordwrap() : error conditions ***\n";

// Zero argument
echo "\n-- Testing wordwrap() function with Zero arguments --\n";
try { var_dump( wordwrap() ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// More than expected number of arguments
echo "\n-- Testing wordwrap() function with more than expected no. of arguments --\n";
$str = 'testing wordwrap function';
$width = 10;
$break = '<br />\n';
$cut = true;
$extra_arg = "extra_arg";

try { var_dump( wordwrap($str, $width, $break, $cut, $extra_arg) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

// $width arg as negative value
echo "\n-- Testing wordwrap() function with negative/zero value for width argument --\n";
echo "-- width = 0 & cut = false --\n";
// width as zero and cut as false
$width = 0;
$cut = false;
var_dump( wordwrap($str, $width, $break, $cut) );

echo "-- width = 0 & cut = true --\n";
// width as zero and cut as true 
$width = 0;
$cut = true;
var_dump( wordwrap($str, $width, $break, $cut) );

echo "-- width = -10 & cut = false --\n";
// width as -ne and cut as false
$width = -10;
$cut = false;
var_dump( wordwrap($str, $width, $break, $cut) );

echo "-- width = -10 & cut = true --\n";
// width as -ne and cut as true 
$width = -10;
$cut = true;
var_dump( wordwrap($str, $width, $break, $cut) );

echo "Done\n";
}
