<?hh

function test($x, $v) :mixed{
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_707() :mixed{
test(true, "");
test(true, "\0foo");
test(new stdClass(), "");
}
