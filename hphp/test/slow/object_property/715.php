<?hh

function test($x, $v) :mixed{
 unset($x->$v);
 var_dump($x);
 }

<<__EntryPoint>>
function main_715() :mixed{
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
}
