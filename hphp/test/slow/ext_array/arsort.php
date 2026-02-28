<?hh


<<__EntryPoint>>
function main_arsort() :mixed{
$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
arsort(inout $fruits);
var_dump($fruits);


$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
ksort(inout $fruits);
var_dump($fruits);


$fruits = dict[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
krsort(inout $fruits);
var_dump($fruits);
}
