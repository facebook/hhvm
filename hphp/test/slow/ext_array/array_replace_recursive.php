<?hh


<<__EntryPoint>>
function main_array_replace_recursive() {
$ar1 = array(
  "color" => darray["favoritte" => "red"],
  5
);
$ar2 = varray[
  10,
  darray["color" => array("favorite" => "green", "blue")]
];
$r = array_replace_recursive($ar1, varray[$ar2]);
var_dump($r);
}
