<?hh
class A {
    private $c;
}

class B extends A {
    private $c;
}

class C extends B {
}
<<__EntryPoint>> function main() {
var_dump(new C);
}
