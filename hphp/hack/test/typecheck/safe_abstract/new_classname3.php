<?hh

<<__ConsistentConstruct>>
abstract class A {}

function example(concrete<classname<A>> $cls): void {
  new $cls(); // ok
}
