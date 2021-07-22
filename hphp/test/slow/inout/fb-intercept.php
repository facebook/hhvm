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

function io_intercept($name, $obj_or_cls, inout $args) {
  var_dump($args);
  $args = varray['red', 'green', 'blue'];
  return shape('value' => null);
}

function main() {
  fb_intercept2('meep', 'io_intercept');
  fb_intercept2('Foo::bar', 'io_intercept');
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, inout $b, inout $c);
  var_dump($a, $b, $c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);
}


<<__EntryPoint>>
function main_fb_intercept() {
main();
}
