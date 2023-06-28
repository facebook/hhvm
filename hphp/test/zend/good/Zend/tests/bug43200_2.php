<?hh

interface A {
    function foo():mixed;
}

abstract class B implements A {
    abstract public function foo():mixed;
}

class C extends B {
    public function foo() :mixed{
        echo 'works';
    }
}
<<__EntryPoint>> function main(): void {
$o = new C();
$o->foo();
}
