<?hh


<<__EntryPoint>>
function main_arsort() {
$fruits = darray[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
arsort(inout $fruits);
var_dump($fruits);


$fruits = darray[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
ksort(inout $fruits);
var_dump($fruits);


$fruits = darray[
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
];
krsort(inout $fruits);
var_dump($fruits);
}
