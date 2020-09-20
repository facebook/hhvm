<?hh

function test($x, $v) {
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_707() {
test(true, "");
test(true, "\0foo");
test(new stdClass(), "");
}
