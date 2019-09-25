<?hh

class A {
  const type T = int;
}
class C {
  const A::T X = "3" as HH\INCORRECT_TYPE<A::T>;
  public A::T $y = "4" as HH\INCORRECT_TYPE<A::T>;
  public static A::T $z = "5" as HH\INCORRECT_TYPE<A::T>;

  public function f(A::T $w = "6" as HH\INCORRECT_TYPE<A::T>): void {
    var_dump($w);
  }
}

const A::T D = "7" as HH\INCORRECT_TYPE<A::T>;

function f(A::T $i = "8" as HH\INCORRECT_TYPE<A::T>): void {
  var_dump($i);
  $j = "9" as HH\INCORRECT_TYPE<A::T>;
  var_dump($j);
}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);
  $c = new C();
  var_dump($c->y);
  var_dump(C::$z);
  $c->f();
  var_dump(D);
  f();
}
