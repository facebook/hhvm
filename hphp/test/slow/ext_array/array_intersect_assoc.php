<?hh


<<__EntryPoint>>
function main_array_intersect_assoc() :mixed{
$array1 = darray[
  "a" => "green",
  "b" => "brown",
  "c" => "blue",
  0 => "red"
];
$array2 = darray[
  "a" => "green",
  0 => "yellow",
  1 => "red"
];
var_dump(array_intersect_assoc($array1, $array2));
}
