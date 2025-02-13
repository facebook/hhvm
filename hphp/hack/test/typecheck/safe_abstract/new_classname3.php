<?hh

<<__ConsistentConstruct>>
abstract class A {}

function example(concreteclassname<A> $cls): void {
  new $cls(); // ok
}
