<?hh


<<__EntryPoint>>
function main_array_uintersect_assoc() {
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
var_dump(array_uintersect_assoc($array1, $array2, fun('strcasecmp')));
}
