<?hh

function f() :mixed{
}
 function g() :mixed{
}

 <<__EntryPoint>>
function main_1729() :mixed{
$t = true;
$a = $t ? f() : g();
var_dump($a);
}
