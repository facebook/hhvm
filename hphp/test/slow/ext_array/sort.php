<?hh


<<__EntryPoint>>
function main_sort() {
$fruits = array("lemon", "orange", "banana", "apple");
sort(inout $fruits);
var_dump($fruits);

$fruits = array("lemon", "orange", "banana", "apple");
rsort(inout $fruits);
var_dump($fruits);
}
