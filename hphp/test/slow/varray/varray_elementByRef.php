<?hh

function main() {
  $a = varray();
  $a[0] = array(0,1,2,3);
  $a[1] = array(5,6,7,8);
  $foo = &$a[0]; // Should warn
  $foo[] = 4;
  $bar = &$a[1]; // Shouldn't warn
  $bar[] = 9;
  var_dump($a);
}

main();
