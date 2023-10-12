<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

// Function type
type Predicate<T> = (function(T): bool);

// Wrap up a function type as a predicate interface
interface IPredicate<-T> {
  public function apply(T $x): bool;
}
final class LambdaPredicate<T> implements IPredicate<T> {
  final public function __construct(private Predicate<T> $lambda): void {}
  public function apply(T $x): bool {
    $l = $this->lambda;
    return ($l)($x);
  }
}

// Generic predicate construction
class P {
  public static function lambda<Tv>(Predicate<Tv> $lambda): IPredicate<Tv> {
    return new LambdaPredicate($lambda);
  }
}

class MyClass {
  public function hello(): string {
    return 'world';
  }
}

function expectsPredicateMyClass(IPredicate<MyClass> $x): IPredicate<MyClass> {
  return $x;
}
function getThing1(): IPredicate<MyClass> {
  // Surrounding context should determine type of $predicate_param
  $var = expectsPredicateMyClass(
    P::lambda(
      function($predicate_param) {
        $predicate_param->hell();
        return true;
      },
    ),
  );
  return $var;
}

function breakIt(): void {
  $x = new MyClass();
  $c = getThing1()->apply($x);
}

function main(): void {
  breakIt();
}
