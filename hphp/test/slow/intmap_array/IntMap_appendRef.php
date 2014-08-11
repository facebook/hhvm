<?hh

function main() {
  $a = miarray();
  $foo = array(1,2,3,4);
  $a[] = &$foo; // Should warn
  $foo[] = "sup";

  $bar = array(5,6,7);
  $a[] = &$bar; // Shouldn't warn
  $bar[] = 8;
  var_dump($a);
}

main();
