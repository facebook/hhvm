<?hh // decl

class Foo {
  static function bar(int $x, inout bool $y, inout string $z) {
    var_dump('inside bar');
    $y = false;
    $z = 'hello-world';
    return $x;
  }
}

function meep(&$f, $g, &$r) {
  var_dump('inside meep');
  $f = 'apple';
  $r = 'orange';
  return $g;
}

function io_intercept($name, $obj_or_cls, inout $args, $ctx, inout $done) {
  var_dump($name);
  var_dump($args);
  $done = false;
}

function main() {
  fb_intercept('meep', 'io_intercept', true);
  fb_intercept('Foo::bar', 'io_intercept', true);
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, &$b, &$c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
}

main();
