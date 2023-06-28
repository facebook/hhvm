<?hh

function f($x) :mixed{ return $x . "my_string"; }
function x($x) :mixed{ return Set { f($x), f($x) }; }

<<__EntryPoint>>
function main_init_mem_leak() :mixed{
var_dump(is_object(x("asd".mt_rand())));
}
