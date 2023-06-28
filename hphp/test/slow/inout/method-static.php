<?hh

class Foo {
  function alpha(inout $a, inout $b) :mixed{
    $a = 'bravo';
    $b = 'charlie';
    return 'delta';
  }
  static function beta(inout $x) :mixed{
    return $x++;
  }
  function one(inout $t) :mixed{
    $t = debug_backtrace()[0]['function'];
  }
  static function two(inout $t) :mixed{
    $t = debug_backtrace()[0]['function'];
  }
}

function main($obj) :mixed{
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


<<__EntryPoint>>
function main_method_static() :mixed{
main(new Foo);
}
