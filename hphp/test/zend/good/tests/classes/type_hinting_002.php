<?hh

class Foo {
    function a(NonExisting $foo) :mixed{}
}
<<__EntryPoint>> function main(): void {
$o = new Foo;
$o->a($o);
}
