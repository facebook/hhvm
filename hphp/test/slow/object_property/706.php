<?hh

function test($x, $v) {
 var_dump($x->$v = 1);
 }

<<__EntryPoint>>
function main_706() {
test(new stdclass, "\0foo");
}
