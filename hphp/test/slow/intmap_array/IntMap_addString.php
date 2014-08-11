<?hh

function main() {
  $a = miarray();
  $a["string"] = 10;
  $a["anotherString"] = 20;
  var_dump($a);

  $a = miarray();
  $foo = "string";
  $a[$foo] = 10;
  $a[$foo] = "something";
  var_dump($a);
}

main();
