<?hh

<<__EntryPoint>>
function main_array_multisort_bug_1() {
error_reporting(-1);
$arr1 = varray[3, 1, 2];
$arr2 = varray['a', 'b', 'c'];
var_dump(array_multisort2(inout $arr1, inout $arr2));
var_dump($arr2);
}
