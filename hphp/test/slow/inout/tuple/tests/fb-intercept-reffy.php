<?hh

class Foo {
  function bar(int $x, inout bool $y, inout string $z) {
    $y = false;
    $z = 'hello-world';
    return $x;
  }
}

function meep(inout $f, $g, inout $r) {
  $f = 'apple';
  $r = 'orange';
  return $g;
}

function io_intercept($name, $obj_or_cls, $args, $ctx, &$done) {
  var_dump($args, $done);
  $args = ['red', 'green', 'blue'];
  $done = $ctx;
}

function main() {
  fb_intercept('meep', 'io_intercept', true);
  fb_intercept('Foo::bar', 'io_intercept', true);
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, inout $b, inout $c);
  var_dump($a, $b, $c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);
}

main();
