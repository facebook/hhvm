<?hh

newtype MyThing = (bool|int);

function foo(MyThing $_): void {}

function bar(arraykey $x): void {
  foo($x);
}
