<?hh

function test($x, $y) :mixed{
$a = vec[$x, $y];
$a[] = 3;
return $a;
}

<<__EntryPoint>>
function main_247() :mixed{
var_dump(test(1,2));
}
