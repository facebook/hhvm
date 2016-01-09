<?hh // strict

class A {
}
class B {
  public static function test1(mixed $x): void {
    var_dump($x instanceof self);
  }
  public static function test2(mixed $x): void {
    var_dump($x instanceof static);
  }
}
class C extends B {
}

trait T {
  require extends A;

  public static function test1(mixed $x): void {
    var_dump($x instanceof self);
  }
  public static function test2(mixed $x): void {
    var_dump($x instanceof static);
  }
  public static function test3(mixed $x): void {
    var_dump($x instanceof parent);
  }
}
class D extends A {
  use T;
}


function test(): void {
  $a = new A();
  $b = new B();
  $c = new C();
  $d = new D();
  $x = 'A';
  var_dump($a instanceof A);
  var_dump($a instanceof B);
  var_dump($a instanceof $a);
  /* HH_FIXME[4026] */
  var_dump($a instanceof $x);
  var_dump($b instanceof B);
  var_dump($b instanceof C);
  var_dump($c instanceof B);
  var_dump($c instanceof C);
  var_dump($c instanceof $b);

  foreach (array($a, $b, $c, $d) as $i) {
    B::test1($i);
    B::test2($i);
    C::test1($i);
    C::test2($i);
    D::test1($i);
    D::test2($i);
    D::test3($i);
  }
}
