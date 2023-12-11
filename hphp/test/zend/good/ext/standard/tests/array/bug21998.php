<?hh
function first_key($a) :mixed{ foreach ($a as $k => $_) { return $k; } }

<<__EntryPoint>> function main(): void {
$a = vec["a", "b", "c"];

var_dump(first_key($a));
var_dump(array_pop(inout $a));
var_dump(first_key($a));
var_dump(array_pop(inout $a));
var_dump(first_key($a));
var_dump(array_pop(inout $a));
var_dump(first_key($a));
}
