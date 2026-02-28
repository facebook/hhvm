<?hh

interface Foo {
    function a(Foo $foo):mixed;
}

interface Bar {
    function b(Bar $bar):mixed;
}

class FooBar implements Foo, Bar {
    function a(Foo $foo) :mixed{
        // ...
    }

    function b(Bar $bar) :mixed{
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
