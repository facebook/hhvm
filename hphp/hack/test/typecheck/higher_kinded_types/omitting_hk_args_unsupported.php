<?hh // strict

function foo<T1, T2<_>>() : void {}

class Foo<T1<_>, T2> {}

function test() : void {
  foo(); // bad, must provide type args explicitly due to HK type
  new Foo(); // bad, same reason

}
