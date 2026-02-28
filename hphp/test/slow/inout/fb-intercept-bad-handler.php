<?hh

class Foo {
  static function bar(int $x, inout bool $y, inout string $z) :mixed{
    $y = false;
    $z = 'hello-world';
    return $x;
  }
}

function meep(inout $f, $g, inout $r) :mixed{
  $f = 'apple';
  $r = 'orange';
  return $g;
}

function io_intercept($name, inout $obj_or_cls, $args) :mixed{
  var_dump($args);
  $args = vec['red', 'green', 'blue'];
  return shape('value' => null);
}

function main() :mixed{
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
function main_fb_intercept_bad_handler() :mixed{
main();
}
