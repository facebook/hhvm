<?hh

function main() {
  $e = keyset[];
  $one = keyset["bar"];
  $two = keyset["apple", "orange"];
  var_dump($one[0]);
  $two[] = "hello";
  var_dump($e, $one, $two);
}

main();
