<?hh

function main() {
  $a = varray();
  $a[0] = 10;
  $foo = $a["0"];
  var_dump($foo);

  $b = varray();
  $b[0] = "0";
  $foo = "0";
  $foo = $b[$foo];
  var_dump($foo);
}

main();
