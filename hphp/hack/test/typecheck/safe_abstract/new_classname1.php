<?hh

<<__ConsistentConstruct>>
abstract class A {}

function example(classname<A> $cls): void {
  new $cls(); // error
}
