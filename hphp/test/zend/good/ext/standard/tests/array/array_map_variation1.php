<?hh

/* Prototype  : array array_map(mixed callback, array input1 [, array input2 ,...])
 * Description: Applies the callback to the elements in given arrays.
 * Source code: ext/standard/array.c
*/

function cb1 ($a) :mixed{return vec[$a];}
function cb2 ($a,$b) :mixed{return vec[$a,$b];}

<<__EntryPoint>> function main(): void {
echo "*** Testing array_map() : string keys ***\n";

$arr = dict["stringkey" => "value"];
var_dump( array_map(cb1<>, $arr));
var_dump( array_map(cb2<>, $arr,$arr));
var_dump( array_map(null,  $arr));
var_dump( array_map(null, $arr, $arr));
echo "Done";
}
