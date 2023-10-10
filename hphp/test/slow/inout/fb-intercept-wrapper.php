<?hh

class Foo {
  static function bar(int $x, inout bool $y, inout string $z) :mixed{
    var_dump('inside bar');
    $y = false;
    $z = 'hello-world';
    return $x;
  }
}

function meep(inout $f, $g, inout $r) :mixed{
  var_dump('inside meep');
  $f = 'apple';
  $r = 'orange';
  return $g;
}

function io_intercept($name, $obj_or_cls, inout $args) :mixed{
  var_dump($name);
  var_dump($args);
  return shape();
}

function main() :mixed{
  fb_intercept2('meep', 'io_intercept');
  fb_intercept2('Foo::bar', 'io_intercept');
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, inout $b, inout $c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
}


<<__EntryPoint>>
function main_fb_intercept_wrapper() :mixed{
main();
}
