<?hh // strict

namespace NS_contravariance_example;

/*
class Cx<T1, -T2, +T3> {
  public function __construct(private T1 $t1, private T2 $t2, private T3 $t3) {}
}
*/

class C<-T> {
  public function __construct(private T $t) {}
}

class Animal {}
class Cat extends Animal {}

function f(C<Cat> $p1): void { var_dump($p1); }

function main(): void {
  // UNSAFE (type error - this is not accepted)
  f(new C(new Animal()));	// accepted
  // UNSAFE (type error - this is not accepted)
  f(new C(new Cat()));
}

/* HH_FIXME[1002] call to main in strict*/
main();
