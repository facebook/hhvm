<?hh

<<__NEVER_INLINE>> function test(...$x) {
  echo "not folding\n";
}

function main() {
  $a = varray[4, 5, 6];
  fb_setprofile('prof');
  test(1,2,3);
  test(...$a);
}

function prof($a, $b, $args) {
  if ($a == 'enter') {
    var_dump($args);
  }
}


<<__EntryPoint>>
function main_profile() {
main();
}
