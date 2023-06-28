<?hh


<<__EntryPoint>>
function main_array_intersect() :mixed{
$array1 = darray[
  "a" => "green",
  0 => "red",
  1 => "blue"
];
$array2 = darray[
  "b" => "green",
  0 => "yellow",
  1 => "red"
];
var_dump(array_intersect($array1, $array2));
}
