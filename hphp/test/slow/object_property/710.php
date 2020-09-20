<?hh

function test($x, $v) {
 var_dump($x->$v++);
 }

<<__EntryPoint>>
function main_710() {
test(new stdclass, "\0foo");
}
