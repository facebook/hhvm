<?hh


<<__EntryPoint>>
function main_array_uintersect_uassoc() :mixed{
$array1 = dict[
  "a" => "green",
  "b" => "brown",
  "c" => "blue",
  "0" => "red"
];
$array2 = dict[
  "a" => "GREEN",
  "B" => "brown",
  "0" => "yellow",
  "1" => "red"
];
var_dump(
  array_uintersect_uassoc($array1, $array2, strcasecmp<>, strcasecmp<>));
}
