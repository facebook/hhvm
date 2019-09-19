<?hh

<<__EntryPoint>>
function main_isset_on_temp() {
var_dump(isset([0, 1][0]));
var_dump(isset(([0, 1] + [])[0]));
var_dump(isset([[0, 1]][0][0]));
var_dump(isset(([[0, 1]] + [])[0][0]));
$o = new stdClass();
$o->a = 'b';
var_dump(isset($o->a));
//var_dump(isset(['a' => 'b']->a));
//var_dump(isset("str"->a));
var_dump(isset((['a' => 'b'] + [])->a));
var_dump(isset((['a' => 'b'] + [])->a->b));
}
