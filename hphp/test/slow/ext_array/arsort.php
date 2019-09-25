<?hh


<<__EntryPoint>>
function main_arsort() {
$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
arsort(inout $fruits);
var_dump($fruits);


$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
ksort(inout $fruits);
var_dump($fruits);


$fruits = array(
  "d" => "lemon",
  "a" => "orange",
  "b" => "banana",
  "c" => "apple"
);
krsort(inout $fruits);
var_dump($fruits);
}
