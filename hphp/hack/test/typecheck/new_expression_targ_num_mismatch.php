<?hh

class Foo<T> {}

function foo<T>() : void {}

function test() : void {
  new Foo<int,int>(); // bad
}
