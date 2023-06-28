<?hh


<<__EntryPoint>>
function main_array_replace_recursive() :mixed{
$ar1 = darray[
  "color" => darray["favoritte" => "red"],
  0 => 5
];
$ar2 = varray[
  10,
  darray["color" => darray["favorite" => "green", 0 => "blue"]]
];
$r = array_replace_recursive($ar1, varray[$ar2]);
var_dump($r);
}
