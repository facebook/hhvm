<?hh


<<__EntryPoint>>
function main_sort() :mixed{
$fruits = varray["lemon", "orange", "banana", "apple"];
sort(inout $fruits);
var_dump($fruits);

$fruits = varray["lemon", "orange", "banana", "apple"];
rsort(inout $fruits);
var_dump($fruits);
}
