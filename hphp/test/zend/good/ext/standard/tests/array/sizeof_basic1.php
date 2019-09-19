<?hh
/* Prototype  : int sizeof(mixed $var)
 * Description: Counts an elements in an array. If Standard PHP library is 
 *              installed, it will return the properties of an object.
 *
 * Alias to functions: count()
 */

/* Testing the sizeof() for some of the scalar types(integer, float) values 
 *
 */ 
<<__EntryPoint>> function main(): void {
echo "*** Testing sizeof() : basic functionality ***\n";

$intval = 10;
$floatval = 10.5;
$stringval = "String";

echo "-- Testing sizeof() for integer type --\n";
echo "default mode: ";
var_dump( sizeof($intval) );
echo "\n";

echo "-- Testing sizeof() for float  type --\n";
echo "default mode: ";
var_dump( sizeof($floatval) );
echo "\n";

echo "Done";
}
