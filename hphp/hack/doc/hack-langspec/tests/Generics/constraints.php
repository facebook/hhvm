<?hh // strict

namespace NS_generics_constraints;

enum E1: int { C1 = 10; C2 = 20; }
class C {}
class CX extends C {}
interface I {}
class D1 implements I {}
class D2 implements I {}
type Point = shape('x' => int, 'y' => int);

class C1a<T as bool> { public function __construct(private T $p1) {} }
class C1b<T as int> {}
class C1c<T as float> {}
class C1d<T as num> {}
class C1e<T as string> {}
class C1f<T as arraykey> {}
class C1g<T as E1> {}
class C1h<T as array<int>> {}
class C1i<T as C> {}
class C1j<T as I> {}
class C1k<T as (int, bool, string)> {}
class C1l<T as Point> {}
class C1m<T as (function (): void)> {}
class C1n<T as ?int> {}
class C1o<T as ?string> {}
class C1p<T as array<int, int>> {}

class C2<T1 as I, T2 as C> {
  public function __construct(T1 $p1, T2 $p2) {
    echo "Inside " . __METHOD__ . "\n";
  }
}

function maxVal<T as num>(T $p1, T $p2): T {
  return $p1 > $p2 ? $p1 : $p2;
}

function main(): void {
  $c1a = new C1a(true);		// OK
  var_dump($c1a);
  $c1a = new C1a(false);		// OK
  var_dump($c1a);
//  $c1a = new C1a(123);		// rejected; violates the constraint

  $c2 = new C2(new D1(), new CX());
  $c2 = new C2(new D2(), new C());

  echo "=============== maxVal ==================\n\n";

  echo "maxVal(10, 20) = " . maxVal(10, 20) . "\n";
  echo "maxVal(15.6, -20.78) = " . maxVal(15.6, -20.78) . "\n";
//  echo "maxVal('red', 'green') = " . maxVal('red', 'green') . "\n";
}

/* HH_FIXME[1002] call to main in strict*/
main();
