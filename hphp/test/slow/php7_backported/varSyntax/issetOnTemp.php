<?hh

<<__EntryPoint>>
function main_isset_on_temp() :mixed{
var_dump(isset(vec[0, 1][0]));
var_dump(isset((vec[0, 1])));
var_dump(isset(vec[vec[0, 1]][0][0]));
var_dump(isset((vec[vec[0, 1]])[0][0]));
$o = new stdClass();
$o->a = 'b';
var_dump(isset($o->a));
//var_dump(isset(['a' => 'b']->a));
//var_dump(isset("str"->a));
var_dump(isset((dict['a' => 'b'])->a));
var_dump(isset((dict['a' => 'b'])->a->b));
}
