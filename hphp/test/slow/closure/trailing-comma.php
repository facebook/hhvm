<?hh


<<__EntryPoint>>
function main_trailing_comma() :mixed{
$f = ($a,) ==> varray[$a];

var_dump($f("Hello World"));
}
