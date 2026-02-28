<?hh

<<__ConsistentConstruct>>
class A {
  const string CNS = "X";
  public static int $prop = 42;
  public static function meth(): void {}
}

class B extends A {
  public function test(classname<A> $a): void {
    $a::meth();
    $a::CNS;
    $a::$prop;
    new $a();
  }
}
