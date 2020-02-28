<?hh

function reverse_strcasecmp($s1,$s2) {
  return strcasecmp($s2,$s1);
}


<<__EntryPoint>>
function main_uasort() {
$fruits = darray[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
uasort(inout $fruits, fun('reverse_strcasecmp'));
var_dump($fruits);

$fruits = darray[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
uksort(inout $fruits, fun('reverse_strcasecmp'));
var_dump($fruits);

uasort(inout $fruits, "undefined_function_");
uksort(inout $fruits, "undefined_function_");
}
