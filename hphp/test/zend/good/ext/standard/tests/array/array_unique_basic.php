<?hh
/* Prototype  : array array_unique(array $input)
 * Description: Removes duplicate values from array
 * Source code: ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_unique() : basic functionality ***\n";

// array with default keys
$input = vec[1, 2, "1", '2'];
var_dump( array_unique($input) );

// associative array
$input = dict["1" => "one", 1 => "one", 2 => "two", '2' => "two"];
var_dump( array_unique($input) );

// mixed array
$input = dict["1" => "one", 0 => "two", 1 => "one", 2 => "two", 3 => "three"];
var_dump( array_unique($input) );

echo "Done";
}
