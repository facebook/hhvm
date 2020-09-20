<?hh

function f($x) {
 $b = $x;
 $b++;
 }

<<__EntryPoint>>
function main_29() {
$a = 1;
f($a);
var_dump($a);
}
