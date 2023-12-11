<?hh


<<__EntryPoint>>
function main_trailing_comma() :mixed{
$f = ($a,) ==> vec[$a];

var_dump($f("Hello World"));
}
