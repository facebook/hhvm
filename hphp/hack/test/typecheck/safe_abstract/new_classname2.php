<?hh

<<__ConsistentConstruct>>
class A {}

function example(classname<A> $cls): void {
  new $cls(); // error because $cls may refer to an abstract subtype of A
}
