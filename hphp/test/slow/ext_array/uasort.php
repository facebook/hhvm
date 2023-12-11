<?hh

function reverse_strcasecmp($s1,$s2) :mixed{
  return strcasecmp($s2,$s1);
}


<<__EntryPoint>>
function main_uasort() :mixed{
$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
uasort(inout $fruits, reverse_strcasecmp<>);
var_dump($fruits);

$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
uksort(inout $fruits, reverse_strcasecmp<>);
var_dump($fruits);

uasort(inout $fruits, "undefined_function_");
uksort(inout $fruits, "undefined_function_");
}
