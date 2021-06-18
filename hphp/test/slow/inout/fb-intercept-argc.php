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

function too_many($name, $obj_or_cls, inout $args, $ctx, inout $done) {
  var_dump($args, $done);
  $args = varray['red', 'green', 'blue', 'apple', 'bannana', 'pear'];
  $done = $ctx;
}

function too_few($name, $obj_or_cls, inout $args, $ctx, inout $done) {
  var_dump($args, $done);
  $args = varray['foo'];
  $done = $ctx;
}

function wrong_type($name, $obj_or_cls, inout $args, $ctx, inout $done) {
  var_dump($args, $done);
  $args = new stdClass;
  $done = $ctx;
}

function main() {
  fb_intercept('meep', 'too_many', true);
  fb_intercept('Foo::bar', 'too_few', true);
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, inout $b, inout $c);
  var_dump($a, $b, $c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);

  fb_intercept('meep', 'wrong_type', true);
  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);
}


<<__EntryPoint>>
function main_fb_intercept_argc() {
main();
}
