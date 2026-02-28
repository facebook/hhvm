<?hh


<<__EntryPoint>>
function main_array_replace_recursive() :mixed{
$ar1 = dict[
  "color" => dict["favoritte" => "red"],
  0 => 5
];
$ar2 = vec[
  10,
  dict["color" => dict["favorite" => "green", 0 => "blue"]]
];
$r = array_replace_recursive($ar1, vec[$ar2]);
var_dump($r);
}
