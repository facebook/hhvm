<?hh
class A {
    private $c;
}

class B extends A {
    private $c;
}

class C extends B {
}
<<__EntryPoint>> function main(): void {
var_dump(new C);
}
