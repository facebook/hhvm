<?hh

abstract final class GStatics {
  public static Vector $x = Vector {};
}
class C {
  public $y = Vector {};
  static public $z = Vector {};
}
class V {
  public $x = dict['a' => 1];
  public $y = Map {'a' => 1};
}
class W extends V {
  public $x = Map {'a' => 1};
  public $y = dict['a' => 1];
}
class X {
  public $prop = vec[Map {'a' => 1}];
}
class Y {
  public $prop = Vector {vec[Map {'a' => 1}]};
}
class Z {
  const FOO = 456;
  const BAR = "yo";
  public $prop = Map {Z::FOO => Z::BAR};
}
class T {
  public $prop = vec[Set {'a'}];
}

<<__EntryPoint>> function main(): void {
  $f = function ($x = Vector {}) {
    return $x;
  };
  $v1 = $f();
  $v2 = $f();
  $v1[] = 11;
  $v2[] = 22;
  $v2[] = 33;
  var_dump(count($v1), count($v2), ($v1 === $v2));

  echo "=========\n";

  $g = function () {
    return GStatics::$x;
  };
  $v1 = $g();
  $v2 = $g();
  $v1[] = 11;
  $v2[] = 22;
  $v2[] = 33;
  var_dump(count($v1), count($v2), ($v1 === $v2));

  echo "=========\n";

  $obj1 = new C;
  $obj2 = new C;
  $v1 = $obj1->y;
  $v2 = $obj2->y;
  $v1[] = 11;
  $v2[] = 22;
  $v2[] = 33;
  var_dump(count($v1), count($v2), ($v1 === $v2));
  var_dump(get_class(C::$z));

  echo "=========\n";

  $obj1 = new W;
  $obj2 = new W;
  var_dump($obj1->x === $obj2->x);
  $obj1->x['b'] = 11;
  $obj2->x['c'] = 22;
  $obj2->x['d'] = 33;
  var_dump(count($obj1->x), count($obj2->x), ($obj1->x === $obj2->x));

  echo "=========\n";

  var_dump($obj1->y === $obj2->y);
  $obj1->y['b'] = 11;
  $obj2->y['c'] = 22;
  $obj2->y['d'] = 33;
  var_dump(count($obj1->y), count($obj2->y), ($obj1->y === $obj2->y));

  echo "=========\n";

  $obj1 = new X;
  $obj2 = new X;
  $obj1->prop[0]['a']++;
  var_dump($obj1->prop[0]['a']);
  var_dump($obj2->prop[0]['a']);
  $obj1 = new Y;
  $obj2 = new Y;
  $obj1->prop[0][0]['a']++;
  var_dump($obj1->prop[0][0]['a']);
  var_dump($obj2->prop[0][0]['a']);

  echo "=========\n";

  $obj1 = new Z;
  var_dump($obj1->prop[Z::FOO], count($obj1->prop));
  $obj2 = new Z;
  $obj1->prop[Z::FOO] = 73;
  var_dump($obj2->prop[Z::FOO]);

  echo "=========\n";

  $obj1 = new T;
  $obj2 = new T;
  $obj1->prop[0][] = 'b';
  var_dump($obj1->prop[0]->contains('b'));
  var_dump($obj2->prop[0]->contains('b'));
}
