<?hh

function numeric_cow($copy) {
  $copy['0'] = 10;
  return $copy;
}

function main() {
  $a = varray();
  $a["0"] = 10;
  $a["1"] = 20;
  var_dump($a);

  $a = varray();
  $foo = "0";
  $a[$foo] = 10;
  $a[$foo] = 20;
  var_dump($a);

  $a = varray();
  $b = numeric_cow($a);
  var_dump($b);
  $a[] = "warning";
}

main();
