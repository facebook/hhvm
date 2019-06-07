<?hh

function f() {
}
 function g() {
}

 <<__EntryPoint>>
function main_1729() {
$t = true;
$a = $t ? f() : g();
var_dump($a);
}
