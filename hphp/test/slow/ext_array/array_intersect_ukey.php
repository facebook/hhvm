<?hh


<<__EntryPoint>>
function main_array_intersect_ukey() :mixed{
$array1 = dict["blue" => 1, "red" => 2, "green" => 3, "purple" => 4];
$array2 = dict["green" => 5, "blue" => 6, "yellow" => 7, "cyan" => 8];
var_dump(array_intersect_ukey($array1, $array2, strcasecmp<>));
}
