<?hh

function test($x, $v) {
 var_dump($x->$v = 1);
 }

<<__EntryPoint>>
function main_705() {
test(new stdClass, "");
}
