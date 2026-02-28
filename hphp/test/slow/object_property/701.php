<?hh

function test($x, $v) :mixed{
 var_dump($x->$v);
 }

<<__EntryPoint>>
function main_701() :mixed{
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
}
