<?hh


<<__EntryPoint>>
function main_array_merge_recursive() :mixed{
$a1 = vec[];
$a2 = dict["key1" => null];
$a1 = array_merge_recursive($a1, $a2);
unset($a1);unset($a2);

$ar1 = dict[
  "color" => dict["favorite" => "red"],
  1 => 5
];
$ar2 = dict[
  "color" => dict["favorite" => "green"],
  0 => "blue"
];

$result = array_merge_recursive($ar1, vec[$ar2]);
var_dump($result);
}
