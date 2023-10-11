<?hh

namespace foo\bar;

<<__ConsistentConstruct>>
class Herp {}

class Derp extends Herp {}

function foo<T as Herp>(classname<T> $class): T {
  return new $class();
}

function bar(Vector<string> $foo): void {}

function main(): void {
  foo(Derp::class);
}
