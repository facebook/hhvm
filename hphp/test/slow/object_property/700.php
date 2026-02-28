<?hh

function test($x, $v) :mixed{
 var_dump(isset($x->$v));
 }

<<__EntryPoint>>
function main_700() :mixed{
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
test(new stdClass, "\0foo");
}
