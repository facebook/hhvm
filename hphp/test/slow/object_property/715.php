<?hh

function test($x, $v) {
 unset($x->$v);
 var_dump($x);
 }

<<__EntryPoint>>
function main_715() {
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
}
