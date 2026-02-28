<?hh
/* Prototype  : int sizeof(mixed $var)
 * Description: Counts an elements in an array. If Standard PHP library is
 *              installed, it will return the properties of an object.
 *
 * Alias to functions: count()
 */

/* Testing the sizeof() for non-scalar type(array) value
 * Sizeof() has been tested for simple integer, string,
 * indexed and mixed arrays.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing sizeof() : basic functionality ***\n";

$int_array = vec[1, 2, 3, 4];
$string_array = vec["Saffron", "White", "Green"];
$indexed_array = dict["Aggression" => "Saffron", "Peace" => "White", "Growth" => "Green"];
$mixed_array = dict[0 => 1, 1 => 2, "Aggression" => "Saffron", 10 => "Ten", "Ten" => 10];

echo "-- Testing sizeof() with integer array --\n";
echo "default mode: ";
var_dump( sizeof($int_array) );
echo "\n";

echo "-- Testing sizeof() with string array --\n";
echo "default mode: ";
var_dump( sizeof($string_array) );
echo "\n";

echo "-- Testing sizeof() with indexed array --\n";
echo "default mode: ";
var_dump( sizeof($indexed_array) );
echo "\n";

echo "-- Testing sizeof() with mixed array --\n";
echo "default mode: ";
var_dump( sizeof($mixed_array) );
echo "\n";

echo "Done";
}
