<?hh

function main() {
  $e = keyset[];
  $one = keyset[1];
  $two = keyset[1, "1"];
  $e[] = 1;
  $e[] = "1";
  $one[] = "1";

  unset($e[1]);
  unset($one[1]);
  unset($two["1"]);
  var_dump($e, $one, $two);
}

main();
