<?hh

function main() {
  $e = keyset[];
  $one = keyset[1];
  $two = keyset[1, "1"];
  $three = keyset[1, 2, 3];
  $e[] = 1;
  $e[] = "1";
  $one[] = "1";

  unset($e[1]);
  unset($one[1]);
  unset($two["1"]);
  unset($three['a']);
  var_dump($e, $one, $two, $three);
}

main();
