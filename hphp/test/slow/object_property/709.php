<?hh

function test($x, $v) {
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_709() {
test(new stdClass, "");
}
