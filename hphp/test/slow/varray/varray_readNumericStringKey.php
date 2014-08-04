<?hh

function main() {
  $a = hphp_varray();
  $a[0] = 10;
  $foo = $a["0"];
  var_dump($foo);

  $b = hphp_varray();
  $b[0] = "0";
  $foo = "0";
  $foo = $b[$foo];
  var_dump($foo);
}

main();
