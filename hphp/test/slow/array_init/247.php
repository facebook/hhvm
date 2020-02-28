<?hh

function test($x, $y) {
$a = varray[$x, $y];
$a[] = 3;
return $a;
}

<<__EntryPoint>>
function main_247() {
var_dump(test(1,2));
}
