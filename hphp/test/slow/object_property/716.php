<?hh

function test($x, $v) {
 unset($x->$v);
 var_dump($x);
 }

<<__EntryPoint>>
function main_716() {
test(new stdclass, "\0foo");
}
