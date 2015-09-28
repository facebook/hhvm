<?hh

namespace foo\bar;

<<__ConsistentConstruct>>
class Herp {
}

class Derp extends Herp { }

type Terp = Derp;

function foo<T as Herp>(classname<T> $class): T { return new $class(); }
function baz<T as Herp>(typename<T> $type): string { return $type; }

function bar(Vector<string> $foo): void {}

function main(): void {
  var_dump(foo(Derp::class));
  var_dump(baz(Terp::class));
}

main();
