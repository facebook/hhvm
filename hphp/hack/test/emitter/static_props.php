<?hh // strict

class A {
  public static int $parent_prop = 100;

  public static function static_fn(): void {
    echo "hi from parent\n";
  }

  public static function call_static(): void {
    self::static_fn();
  }
  public static function call_static_statically(): void {
    static::static_fn();
  }
}

class B extends A { }

trait T {
  require extends A;
  public static function trait_fn2(): void {
    self::call_static();
    parent::call_static();
  }
  public static function trait_fn(): void {
    self::trait_fn2();
  }
}

class C extends B {
  use T;

  public static int $static_prop = 100;

  public static array<int> $array_prop = array();

  public function get_self_prop(): int {
    return self::$static_prop;
  }
  public function get_static_prop(): int {
    return static::$static_prop;
  }
  public function get_parent_prop(): int {
    return parent::$parent_prop;
  }
  public function get_this_prop(): int {
    return $this::$static_prop;
  }

  public static function static_fn(): void {
    echo "hi from child\n";
  }
}

class Cns {
  const C = 5;
}
class D {
  public static int $sprop = Cns::C;
}

function test(): void {
  $x = C::$static_prop;
  var_dump($x);
  C::$static_prop = 59;
  C::$static_prop++;
  var_dump(C::$static_prop);

  C::$array_prop[0] = 8;
  C::$array_prop[1] = 3;
  C::$array_prop[2] = 10;
  C::$array_prop[2] += (++C::$array_prop[0]) * C::$array_prop[1];

  var_dump(C::$array_prop[2]);
  var_dump(C::$array_prop);

  $n = 0;
  C::$array_prop[$n+1];
  C::$array_prop[$n+1] = 10;

  var_dump(C::$array_prop);

  $c = new C();
  var_dump($c->get_self_prop());
  var_dump($c->get_parent_prop());
  var_dump($c->get_static_prop());
  var_dump($c->get_this_prop());
  $c::static_fn();

  C::static_fn();
  C::call_static();
  C::call_static_statically();
  A::call_static_statically();
  C::trait_fn();

  var_dump(D::$sprop);
}
