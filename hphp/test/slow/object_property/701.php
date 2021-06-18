<?hh

function test($x, $v) {
 var_dump($x->$v);
 }

<<__EntryPoint>>
function main_701() {
test(false, "");
test(false, "\0foo");
test(true, "");
test(true, "\0foo");
test(new stdClass, "");
}
