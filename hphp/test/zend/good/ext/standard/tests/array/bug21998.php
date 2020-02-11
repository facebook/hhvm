<?hh
<<__EntryPoint>> function main(): void {
$a = varray["a", "b", "c"];

var_dump(key($a));
var_dump(array_pop(inout $a));
var_dump(key($a));
var_dump(array_pop(inout $a));
var_dump(key($a));
var_dump(array_pop(inout $a));
var_dump(key($a));
}
