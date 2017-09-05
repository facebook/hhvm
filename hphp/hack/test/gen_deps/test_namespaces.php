<?hh // strict
namespace B {
  class Foo {}
  function test () : Foo {
    bar();
    baz();
    // UNSAFE
  }
  function baz () : void {
  }
}


function bar() : void {
}
