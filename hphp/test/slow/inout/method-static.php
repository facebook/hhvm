<?hh

class Foo {
  function alpha(inout $a, inout $b) {
    $a = 'bravo';
    $b = 'charlie';
    return 'delta';
  }
  static function beta(inout $x) {
    return $x++;
  }
  function one(inout $t) {
    $t = debug_backtrace()[0]['function'];
  }
  static function two(inout $t) {
    $t = debug_backtrace()[0]['function'];
  }
}

function main($obj) {
  $a = 'beta';
  $b = 'gamma';
  $x = 41;
  $r1 = $obj->alpha(inout $a, inout $b);
  $r2 = Foo::beta(inout $x);
  var_dump($a, $b, $x, $r1, $r2);

  $t = null;
  $obj->one(inout $t);
  var_dump($t);

  Foo::two(inout $t);
  var_dump($t);
}

main(new Foo);
