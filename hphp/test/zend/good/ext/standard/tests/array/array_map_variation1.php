<?hh

/* Prototype  : array array_map(mixed callback, array input1 [, array input2 ,...])
 * Description: Applies the callback to the elements in given arrays.
 * Source code: ext/standard/array.c
*/

function cb1 ($a) {return array ($a);};
function cb2 ($a,$b) {return array ($a,$b);};

<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : string keys ***\n";

$arr = array("stringkey" => "value");
var_dump( array_map(fun("cb1"), $arr));
var_dump( array_map(fun("cb2"), $arr,$arr));
var_dump( array_map(null,  $arr));
var_dump( array_map(null, $arr, $arr));
echo "Done";
}
