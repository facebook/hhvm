<?hh


<<__EntryPoint>>
function main_trailing_comma() {
$f = ($a,) ==> varray[$a];

var_dump($f("Hello World"));
}
