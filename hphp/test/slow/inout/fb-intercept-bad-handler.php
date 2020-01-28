<?hh

class Foo {
  static function bar(int $x, inout bool $y, inout string $z) {
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

function io_intercept($name, inout $obj_or_cls, $args, $ctx, inout $done) {
  var_dump($args, $done);
  $args = varray['red', 'green', 'blue'];
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


<<__EntryPoint>>
function main_fb_intercept_bad_handler() {
main();
}
