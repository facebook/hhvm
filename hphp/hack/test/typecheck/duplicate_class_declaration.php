<?hh

class Foo { public function foo(): void {} }
class Foo { public function bar(): void {} } // error

function f(): void {
  $foo = new Foo();
  $foo->foo(); // ok
  $foo->bar(); // error
}
