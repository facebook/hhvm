<?hh

newtype MyThing = (bool|int);

function foo(MyThing $_): void {}

function bar(?string $x): void {
  foo($x);
}
