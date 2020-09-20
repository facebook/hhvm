<?hh
class Foo {
}

use A\B as Foo;
<<__EntryPoint>> function main(): void {
new Foo();
}
