<?hh
/*
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing array_splice() basic operations ***\n";
echo "test truncation \n";
$input = vec["red", "green", "blue", "yellow"];
var_dump (array_splice(inout $input, 2));
var_dump ($input);
// $input is now array("red", "green")

echo "test removing entries from the middle \n";
$input = vec["red", "green", "blue", "yellow"];
var_dump (array_splice(inout $input, 1, -1));
var_dump ($input);
// $input is now array("red", "yellow")

echo "test substitution at end \n";
$input = vec["red", "green", "blue", "yellow"];
var_dump (array_splice(inout $input, 1, count($input), "orange"));
var_dump ($input);
// $input is now array("red", "orange")

$input = vec["red", "green", "blue", "yellow"];
var_dump (array_splice(inout $input, -1, 1, vec["black", "maroon"]));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "black", "maroon")

echo "test insertion \n";
$input = vec["red", "green", "blue", "yellow"];
var_dump (array_splice(inout $input, 3, 0, "purple"));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "purple", "yellow");
}
