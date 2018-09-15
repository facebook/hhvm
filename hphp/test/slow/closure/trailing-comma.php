<?hh


<<__EntryPoint>>
function main_trailing_comma() {
$f = ($a,) ==> array($a);

var_dump($f("Hello World"));
}
