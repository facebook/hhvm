<?hh


<<__EntryPoint>>
function main_array_combine() {
$a = varray["green", "red", "yellow"];
$b = varray["avocado", "apple", "banana"];
$c = array_combine($a, $b);
var_dump($c);
}
