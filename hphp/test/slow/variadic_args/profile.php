<?hh

function test(...$x) {
}

function main() {
  $a = array(4, 5, 6);
  fb_setprofile('prof');
  test(1,2,3);
  test(...$a);
}

function prof($a, $b, $args) {
  if ($a == 'enter') {
    var_dump($args);
  }
}

main();
