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
  function info(inout $x, inout $y) :mixed{
    $x = self::class;
    $y = static::class;
    return $this->beep();
  }
}

class Bar extends Foo {
  function beep() :mixed{ return 42; }
  function info2(inout $x, inout $y, inout $z) :mixed{
    $z = parent::class;
    return parent::info(inout $x, inout $y);
  }
}

function main($obj, $foo, $alpha, $beta, $one, $two, $bar, $arr1, $arr2) :mixed{
  $a = 'beta';
  $b = 'gamma';
  $x = 41;
  $r1 = $obj->$alpha(inout $a, inout $b);
  $r2 = $foo::$beta(inout $x);
  var_dump($a, $b, $x, $r1, $r2);

  $t = null;
  $obj->$one(inout $t);
  var_dump($t);

  $foo::$two(inout $t);
  var_dump($t);

  $q = null;
  Foo::$two(inout $q);
  var_dump($q);

  $r = null;
  $foo::two(inout $r);
  var_dump($r);

  $self = null;
  $static = null;
  $parent = null;
  $num = $bar->info2(inout $self, inout $static, inout $parent);
  var_dump($num, $self, $static, $parent);

  $a = null;
  $arr1(inout $a);
  var_dump($a);
  $arr2(inout $a);
  var_dump($a);
}


<<__EntryPoint>>
function main_method_dynamic() :mixed{
main(new Foo, 'Foo', 'alpha', 'beta', 'one', 'two', new Bar,
     vec[new Foo, 'one'], vec['Foo', 'two']);
}
