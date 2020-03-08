<?hh // strict

namespace NS_covariance_example;

//class C<T> { // equivalent to the following line, as type parameters are covariant by default
class C<+T> {
  public function __construct(private T $t) {}
}

class Animal {}
class Cat extends Animal {}

function f(C<Animal> $p1): void { var_dump($p1); }

function g(array<Animal> $p1): void { var_dump($p1); }

function main(): void {
  f(new C(new Animal()));
  f(new C(new Cat()));	// accepted

  g(array(new Animal(), new Animal()));
  g(array(new Cat(), new Cat(), new Animal()));	// arrays are covariant
}

/* HH_FIXME[1002] call to main in strict*/
main();
