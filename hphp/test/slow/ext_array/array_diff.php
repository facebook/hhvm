<?hh


<<__EntryPoint>>
function main_array_diff() :mixed{
$array1 = darray[
  "a" => "green",
  0 => "red",
  1 => "blue",
  2 => "red"
];
$array2 = darray[
  "b" => "green",
  0 => "yellow",
  1 => "red"
];

$result = array_diff($array1, $array2);
var_dump($result);

$a = varray["b"];
$b = varray["b", "c"];
var_dump(array_diff($b, $a));
}
