<?hh


<<__EntryPoint>>
function main_reset() {
$array = varray["step one", "step two", "step three", "step four"];

// by default, the pointer is on the first element
var_dump(current($array));

// skip two steps
next(inout $array);
next(inout $array);
var_dump(current($array));

// reset pointer, start again on step one
reset(inout $array);
var_dump(current($array));
}
