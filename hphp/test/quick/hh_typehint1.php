<?hh

class Bar {}
class Foo {}
function foo(?Bar $x) :mixed{}
<<__EntryPoint>> function main(): void {
foo(new Foo);
}
