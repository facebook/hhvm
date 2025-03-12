<?hh

<<__ConsistentConstruct>>
abstract class A {}

function example(class<A> $cls): void {
  new $cls(); // error
}
