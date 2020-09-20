<?hh // strict

class Foo<T> {}

function foo<T>(T $x) : void {}

function test() : void {
  foo<Foo>(); // bad, missing type arg on Foo
}
