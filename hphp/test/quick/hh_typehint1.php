<?hh

class Bar {}
class Foo {}
function foo(?Bar $x) {}
<<__EntryPoint>> function main(): void {
foo(new Foo);
}
