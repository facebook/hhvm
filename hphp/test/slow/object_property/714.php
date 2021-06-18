<?hh

function test($x, $v) {
 var_dump($x->$v += 1);
 }

<<__EntryPoint>>
function main_714() {
test(new stdClass, "\0foo");
}
