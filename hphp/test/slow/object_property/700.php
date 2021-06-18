<?hh

function test($x, $v) {
 var_dump(isset($x->$v));
 }

<<__EntryPoint>>
function main_700() {
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
test(new stdClass, "\0foo");
}
