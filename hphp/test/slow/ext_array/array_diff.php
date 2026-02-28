<?hh


<<__EntryPoint>>
function main_array_diff() :mixed{
$array1 = dict[
  "a" => "green",
  0 => "red",
  1 => "blue",
  2 => "red"
];
$array2 = dict[
  "b" => "green",
  0 => "yellow",
  1 => "red"
];

$result = array_diff($array1, $array2);
var_dump($result);

$a = vec["b"];
$b = vec["b", "c"];
var_dump(array_diff($b, $a));
}
