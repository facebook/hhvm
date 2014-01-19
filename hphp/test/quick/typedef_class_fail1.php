<?hh

class MyClass {}

type Foo = MyClass;

function blah(): void {
  $x = new Foo();  // Error, new types don't create new value constructors
}
blah();
