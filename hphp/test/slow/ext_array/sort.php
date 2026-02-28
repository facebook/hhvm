<?hh


<<__EntryPoint>>
function main_sort() :mixed{
$fruits = vec["lemon", "orange", "banana", "apple"];
sort(inout $fruits);
var_dump($fruits);

$fruits = vec["lemon", "orange", "banana", "apple"];
rsort(inout $fruits);
var_dump($fruits);
}
