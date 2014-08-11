<?hh

function numeric_cow($copy) {
  $copy['123'] = 10;
  return $copy;
}

function main() {
  $a = miarray();
  $a["123"] = 10;
  $a["456"] = 20;
  var_dump($a);

  $a = miarray();
  $foo = "123";
  $a[$foo] = 10;
  $a[$foo] = 20;
  var_dump($a);

  $a = miarray();
  $b = numeric_cow($a);
  var_dump($b);
  $a[] = "warning";
}

main();
