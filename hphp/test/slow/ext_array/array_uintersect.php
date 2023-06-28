<?hh


<<__EntryPoint>>
function main_array_uintersect() :mixed{
$array1 = darray[
  "a" => "green",
  "b" => "brown",
  "c" => "blue",
  0 => "red"
];
$array2 = darray[
  "a" => "GREEN",
  "B" => "brown",
  0 => "yellow",
  1 => "red"
];
var_dump(array_uintersect($array1, $array2, strcasecmp<>));
}
