<?hh

function test($x, $v) {
 var_dump($x->$v);
 }

<<__EntryPoint>>
function main_702() {
test(new stdclass, "\0foo");
}
