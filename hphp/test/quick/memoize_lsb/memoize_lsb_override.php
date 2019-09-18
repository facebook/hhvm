<?hh

<<__ConsistentConstruct>>
class A {
  public static function instance(): this {
    return new A();
  }
}

class B extends A {
  <<__MemoizeLSB>>
  public static function instance(): this {
    return new static();
  }
}

class C extends B {
}

class D extends C {
  public static this $sideEffect;

  <<__Memoize>>
  public static function instance(): this {
    self::$sideEffect = new D();
    return self::$sideEffect;
  }
}

class E extends D {
  <<__MemoizeLSB>>
  public static function instance(): this {
    return new static();
  }
}

class F extends E {
}

class G extends F {
  public static function instance(): this {
    return new static();
  }
}

<<__EntryPoint>> function main(): void {
$a = A::instance();
$b = B::instance();
$c = C::instance();
$d = D::instance();
$e = E::instance();
$f = F::instance();
$g = G::instance();

var_dump($a);
var_dump($b);
var_dump($c);
var_dump($d);
var_dump($e);
var_dump($f);
var_dump($g);

var_dump($a === A::instance());
var_dump($b === B::instance());
var_dump($c === C::instance());
var_dump($d === D::instance());
var_dump($e === E::instance());
var_dump($f === F::instance());
var_dump($g === G::instance());

var_dump(D::$sideEffect === $d);
}
