<?hh

function main() {
  $a = msarray();
  $a['a'] = array(0,1,2,3);
  $a['b'] = array(5,6,7,8);
  $foo = &$a['a']; // Should warn
  $foo[] = 4;
  $bar = &$a['b']; // Shouldn't warn
  $bar[] = 9;
  var_dump($a);
}

main();
