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

function too_many($name, $obj_or_cls, inout $args) :mixed{
  var_dump($args);
  $args = vec['red', 'green', 'blue', 'apple', 'bannana', 'pear'];
  return shape('value' => null);
}

function too_few($name, $obj_or_cls, inout $args) :mixed{
  var_dump($args);
  $args = vec['foo'];
  return shape('value' => null);
}

function wrong_type($name, $obj_or_cls, inout $args) :mixed{
  var_dump($args);
  $args = new stdClass;
  return shape('value' => null);
}

function main() :mixed{
  fb_intercept2('meep', 'too_many');
  fb_intercept2('Foo::bar', 'too_few');
  $a = 1; $b = true; $c = 'c';
  Foo::bar($a, inout $b, inout $c);
  var_dump($a, $b, $c);

  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);

  fb_intercept2('meep', 'wrong_type');
  $a = 1; $b = true; $c = 'c';
  meep(inout $a, $b, inout $c);
  var_dump($a, $b, $c);
}


<<__EntryPoint>>
function main_fb_intercept_argc() :mixed{
main();
}
