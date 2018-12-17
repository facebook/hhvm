<?hh


<<__EntryPoint>>
function main() {
  $a = dict["1" => 42];
  var_dump(array_unshift(&$a, 2));
  var_dump($a);
}
