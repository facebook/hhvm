<?hh

function main() {
  $a = varray();
  $foo = array(1,2,3,4);
  $a[0] = &$foo; // Should warn
  $foo[] = "sup";

  $bar = array(5,6,7);
  $a[1] = &$bar; // Shouldn't warn
  $bar[] = 8;
  var_dump($a);
}

main();
