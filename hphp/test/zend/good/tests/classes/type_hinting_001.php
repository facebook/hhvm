<?hh

interface Foo {
    function a(Foo $foo);
}

interface Bar {
    function b(Bar $bar);
}

class FooBar implements Foo, Bar {
    function a(Foo $foo) {
        // ...
    }

    function b(Bar $bar) {
        // ...
    }
}

class Blort {
}
<<__EntryPoint>> function main(): void {
$a = new FooBar;
$b = new Blort;

$a->a($b);
$a->b($b);
}
