<?hh


<<__EntryPoint>>
function main_array_unshift() :mixed{
$q = varray["orange", "banana"];
array_unshift(inout $q, "apple", "raspberry");
var_dump($q);

$q = darray[0 => "orange", 1 => "banana", "a" => "dummy"];
unset($q['a']);
array_unshift(inout $q, "apple", "raspberry");
var_dump($q);
}
