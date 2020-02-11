<?hh
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array 
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unique() : basic functionality ***\n";

// array with default keys
$input = varray[1, 2, "1", '2'];
var_dump( array_unique($input) );

// associative array
$input = darray["1" => "one", 1 => "one", 2 => "two", '2' => "two"];
var_dump( array_unique($input) );

// mixed array
$input = array("1" => "one", "two", "one", 2 => "two", "three");
var_dump( array_unique($input) );

echo "Done";
}
