<?hh
class Base {}
class Derived extends Base {
  public function __construct(public int $v) {}
}
class Cov<+T> {
  public static function foo(T $v): void {}
}
class Concrete extends Cov<Derived> {
  <<__Override>>
  public static function foo(Derived $v): void {
    echo $v->v;
  }
}

class Violate {
  public static function foo(classname<Concrete> $z): void {
    self::bar($z);
  }
  public static function bar(classname<Cov<Base>> $y): void {
    $y::foo(new Base()); // Concrete::foo tries to coax an `int $v` out of Base!
    // It's not very effective.
  }
}

//Violate::foo(Concrete::class);
