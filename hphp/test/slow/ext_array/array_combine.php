<?hh


<<__EntryPoint>>
function main_array_combine() :mixed{
$a = vec["green", "red", "yellow"];
$b = vec["avocado", "apple", "banana"];
$c = array_combine($a, $b);
var_dump($c);
}
