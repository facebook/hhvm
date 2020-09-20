<?hh

interface A {
    function foo();
}

abstract class B implements A {
    abstract public function foo();
}

class C extends B {
    public function foo() {
        echo 'works';
    }
}
<<__EntryPoint>> function main(): void {
$o = new C();
$o->foo();
}
